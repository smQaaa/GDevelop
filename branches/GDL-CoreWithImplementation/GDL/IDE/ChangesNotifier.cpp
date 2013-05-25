/** \file
 *  Game Develop
 *  2008-2013 Florian Rival (Florian.Rival@gmail.com)
 */
#if defined(GD_IDE_ONLY)
#include <string>
#include <vector>
#include <iostream>
#include <wx/datetime.h>
#include "ChangesNotifier.h"
#include "GDCore/IDE/EventsChangesNotifier.h"
#include "GDCore/PlatformDefinition/Project.h"
#include "GDCore/PlatformDefinition/Layout.h"
#include "GDCore/PlatformDefinition/ExternalEvents.h"
#include "GDL/CppPlatform.h"
#include "GDL/IDE/DependenciesAnalyzer.h"
#include "GDL/Scene.h"
#include "GDL/Project.h"
#include "GDL/Events/CodeCompilationHelpers.h"

void ChangesNotifier::OnObjectEdited(gd::Project & game, gd::Layout * scene, gd::Object & object) const
{
    if ( scene )
        scene->SetRefreshNeeded();
    else //Scene pointer is not NULL: Update shared data of all scenes
    {
        for (unsigned int i = 0;i<game.GetLayoutCount();++i)
            game.GetLayout(i).SetRefreshNeeded();
    }
}

void ChangesNotifier::OnObjectAdded(gd::Project & project, gd::Layout * layout, gd::Object & object) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnObjectRenamed(gd::Project & project, gd::Layout * layout, gd::Object & object, const std::string & oldName) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnVariablesModified(gd::Project & project, gd::Layout * layout) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnObjectGroupAdded(gd::Project & project, gd::Layout * layout, const std::string & groupName) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnObjectGroupEdited(gd::Project & project, gd::Layout * layout, const std::string & groupName) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnObjectGroupRenamed(gd::Project & project, gd::Layout * layout, const std::string & groupName, const std::string & oldName) const
{
    RequestFullRecompilation(project, layout);
}
void ChangesNotifier::OnObjectGroupDeleted(gd::Project & project, gd::Layout * layout, const std::string & groupName) const
{
    RequestFullRecompilation(project, layout);
}

void ChangesNotifier::OnObjectsDeleted(gd::Project & project, gd::Layout * layout, const std::vector<std::string> & deletedObjects) const
{
    RequestAutomatismsSharedDataUpdate(project, layout);
    RequestFullRecompilation(project, layout);
}

void ChangesNotifier::OnAutomatismEdited(gd::Project & game, gd::Layout * scene, gd::Object & object, gd::Automatism & automatism) const
{
    if ( scene )
        scene->SetRefreshNeeded();
    else //Scene pointer is not NULL: Update shared data of all scenes
    {
        for (unsigned int i = 0;i<game.GetLayoutCount();++i)
            game.GetLayout(i).SetRefreshNeeded();
    }
}

void ChangesNotifier::OnAutomatismAdded(gd::Project & project, gd::Layout * layout, gd::Object & object, gd::Automatism & automatism) const
{
    RequestAutomatismsSharedDataUpdate(project, layout);
    RequestFullRecompilation(project, layout);
}

void ChangesNotifier::OnAutomatismRenamed(gd::Project & project, gd::Layout * layout, gd::Object & object, gd::Automatism & automatism, const std::string & oldName) const
{
    RequestAutomatismsSharedDataUpdate(project, layout);
    RequestFullRecompilation(project, layout);
}

void ChangesNotifier::OnAutomatismDeleted(gd::Project & project, gd::Layout * layout, gd::Object & object, const std::string & automatismName) const
{
    RequestAutomatismsSharedDataUpdate(project, layout);
    RequestFullRecompilation(project, layout);
}

void ChangesNotifier::OnObjectVariablesChanged(gd::Project & game, gd::Layout * scene, gd::Object & object) const
{
    if ( scene )
        scene->SetRefreshNeeded();
    else //Scene pointer is NULL: Update shared data of all scenes
    {
        for (unsigned int i = 0;i<game.GetLayoutCount();++i)
        {
            game.GetLayout(i).SetRefreshNeeded();
            game.GetLayout(i).SetCompilationNeeded();
        }
        for (unsigned int i = 0;i<game.GetExternalEventsCount();++i)
        {
            game.GetExternalEvents(i).SetLastChangeTimeStamp(wxDateTime::Now().GetTicks()); //Do no forget external events as they can have been compiled separately from scenes.
        }
    }
}

void ChangesNotifier::OnEventsModified(gd::Project & game, gd::Layout & scene, bool indirectChange, std::string sourceOfTheIndirectChange) const
{
    scene.SetRefreshNeeded();
    if ( !indirectChange ) //Changes occured directly in the scene: Recompile it.
    {
        scene.SetCompilationNeeded();
        CodeCompilationHelpers::CreateSceneEventsCompilationTask(game, scene);
    }
    else
    {
        DependenciesAnalyzer analyzer(game);
        if ( analyzer.ExternalEventsCanBeCompiledForAScene(sourceOfTheIndirectChange) == scene.GetName() )
        {
            //Do nothing: Changes occured in an external event which is compiled separately
        }
        else
        {
            //Changes occurred in an external event which is directly included in the scene events.
            scene.SetCompilationNeeded();
            CodeCompilationHelpers::CreateSceneEventsCompilationTask(game, scene);
        }
    }
}

void ChangesNotifier::OnEventsModified(gd::Project & game, gd::ExternalEvents & events, bool indirectChange, std::string sourceOfTheIndirectChange) const
{
    DependenciesAnalyzer analyzer(game);
    std::string associatedScene = analyzer.ExternalEventsCanBeCompiledForAScene(events.GetName());
    bool externalEventsAreCompiledSeparately = !associatedScene.empty();

    if ( externalEventsAreCompiledSeparately )
    {
        //The external events are compiled separately from the scene events:
        //We need to recompile them if the changes occured inside them.

        if ( !indirectChange )
        {
            DependenciesAnalyzer analyzer(game);
            if ( analyzer.ExternalEventsCanBeCompiledForAScene(sourceOfTheIndirectChange) == associatedScene )
            {
                //Do nothing: Changes occurred in an external event which is compiled separately
            }
            else
            {
                //Changes occurred in an another external event which is directly included in our external events.
                events.SetLastChangeTimeStamp(wxDateTime::Now().GetTicks());
                CodeCompilationHelpers::CreateExternalEventsCompilationTask(game, events);
            }
        }
        else
        {
            //Changes occurred directly inside the external events: We need to recompile them
            events.SetLastChangeTimeStamp(wxDateTime::Now().GetTicks());
            CodeCompilationHelpers::CreateExternalEventsCompilationTask(game, events);
        }

    }
}

void ChangesNotifier::OnLayoutAdded(gd::Project & project, gd::Layout & layout) const
{
    //A new layout may trigger recompilation of some events.
    gd::EventsChangesNotifier::NotifyChangesInEventsOfScene(CppPlatform::Get(), project, layout);
}

void ChangesNotifier::OnLayoutRenamed(gd::Project & project, gd::Layout & layout, const std::string & oldName) const
{
    //A renamed layout may trigger recompilation of some events.
    gd::EventsChangesNotifier::NotifyChangesInEventsOfScene(CppPlatform::Get(), project, layout);
}

void ChangesNotifier::OnExternalEventsAdded(gd::Project & project, gd::ExternalEvents & events) const
{
    //New external events may trigger recompilation of some events.
    gd::EventsChangesNotifier::NotifyChangesInEventsOfExternalEvents(CppPlatform::Get(), project, events);
}

void ChangesNotifier::OnExternalEventsRenamed(gd::Project & project, gd::ExternalEvents & events, const std::string & oldName) const
{
    //A renamed external events sheet may trigger recompilation of some events.
    gd::EventsChangesNotifier::NotifyChangesInEventsOfExternalEvents(CppPlatform::Get(), project, events);
}

void ChangesNotifier::OnExternalEventsDeleted(gd::Project & project, const std::string deletedLayout) const
{
    RequestFullRecompilation(project, NULL);
}

void ChangesNotifier::OnLayoutDeleted(gd::Project & project, const std::string deletedLayout) const
{
    //There is a possibility that the deleting the layout now enables some external events
    //to be compiled separately from the scene using them ( For instance, scenes A and B are including
    //the external events, and scene B is deleted ).
    //So we request the recompilation of all scenes.
    RequestFullRecompilation(project, NULL);
}

void ChangesNotifier::RequestFullRecompilation(gd::Project & game, gd::Layout * scene) const
{
    if ( scene )
    {
        //Notify the scene it has been changed...
        scene->SetRefreshNeeded();

        //...as well as the dependencies
        DependenciesAnalyzer analyzer(game);
        analyzer.Analyze(scene->GetEvents());
        std::set< std::string > externalEventsDependencies = analyzer.GetExternalEventsDependencies();
        for (std::set<std::string>::const_iterator i = externalEventsDependencies.begin();i!=externalEventsDependencies.end();++i)
        {
            if ( game.HasExternalEventsNamed(*i) )
                game.GetExternalEvents(*i).SetLastChangeTimeStamp(wxDateTime::Now().GetTicks());
        }

        //And ask for a recompilation of everything.
        CodeCompilationHelpers::CreateSceneEventsCompilationTask(game, *scene);
    }
    else //Scene pointer is NULL: Mark all scenes as modified
    {
        for (unsigned int i = 0;i<game.GetLayoutCount();++i)
        {
            game.GetLayout(i).SetRefreshNeeded();
            game.GetLayout(i).SetCompilationNeeded();
        }
        for (unsigned int i = 0;i<game.GetExternalEventsCount();++i)
        {
            game.GetExternalEvents(i).SetLastChangeTimeStamp(wxDateTime::Now().GetTicks()); //Do no forget external events as they can have been compiled separately from scenes.
        }
    }
}

void ChangesNotifier::RequestAutomatismsSharedDataUpdate(gd::Project & game, gd::Layout * scene) const
{
    if ( scene )
        scene->UpdateAutomatismsSharedData(game);
    else //Scene pointer is NULL: Update shared data of all scenes
    {
        for (unsigned int i = 0;i<game.GetLayoutCount();++i)
            game.GetLayout(i).UpdateAutomatismsSharedData(game);
    }
}

void ChangesNotifier::OnResourceModified(gd::Project & project, const std::string & resourceName) const
{
    project.imagesChanged.push_back(resourceName);
}

#endif
