// @flow
import { Trans } from '@lingui/macro';

import React, { Component } from 'react';
import Dialog from '../../UI/Dialog';
import FlatButton from '../../UI/FlatButton';
import RaisedButton from '../../UI/RaisedButton';
import { sendExportLaunched } from '../../Utils/Analytics/EventSender';
import { Column, Line, Spacer } from '../../UI/Grid';
import { showErrorBox } from '../../UI/Messages/MessageBox';
import { findGDJS } from './LocalGDJSFinder';
import localFileSystem from './LocalFileSystem';
import LocalFolderPicker from '../../UI/LocalFolderPicker';
import HelpButton from '../../UI/HelpButton';
import {
  displayProjectErrorsBox,
  getErrors,
} from '../../ProjectManager/ProjectErrorsChecker';
import assignIn from 'lodash/assignIn';
import optionalRequire from '../../Utils/OptionalRequire';
import Window from '../../Utils/Window';
import Text from '../../UI/Text';
const electron = optionalRequire('electron');
const shell = electron ? electron.shell : null;

const gd = global.gd;

type Props = {|
  project: gdProject,
|};

type State = {|
  outputDir: string,
  exportFinishedDialogOpen: boolean,
|};

class LocalCordovaExport extends Component<Props, State> {
  state = {
    exportFinishedDialogOpen: false,
    outputDir: '',
  };

  componentDidMount() {
    const { project } = this.props;
    this.setState({
      outputDir: project ? project.getLastCompilationDirectory() : '',
    });
  }

  static prepareExporter = (): Promise<any> => {
    return findGDJS().then(({ gdjsRoot }) => {
      console.info('GDJS found in ', gdjsRoot);

      const fileSystem = assignIn(
        new gd.AbstractFileSystemJS(),
        localFileSystem
      );
      const exporter = new gd.Exporter(fileSystem, gdjsRoot);

      return {
        exporter,
      };
    });
  };

  launchExport = () => {
    const t = str => str; //TODO;
    const { project } = this.props;
    if (!project) return;

    sendExportLaunched('local-cordova');

    if (!displayProjectErrorsBox(t, getErrors(t, project))) return;

    const outputDir = this.state.outputDir;
    project.setLastCompilationDirectory(outputDir);

    LocalCordovaExport.prepareExporter()
      .then(({ exporter }) => {
        const exportOptions = new gd.MapStringBoolean();
        exportOptions.set('exportForCordova', true);
        exporter.exportWholePixiProject(project, outputDir, exportOptions);
        exportOptions.delete();
        exporter.delete();

        this.setState({
          exportFinishedDialogOpen: true,
        });
      })
      .catch(err => {
        showErrorBox('Unable to export the game', err);
      });
  };

  openExportFolder = () => {
    if (shell) shell.openItem(this.state.outputDir);
  };

  openPhoneGapBuild = () => {
    Window.openExternalURL('https://build.phonegap.com');
  };

  render() {
    const t = str => str; //TODO;
    const { project } = this.props;
    if (!project) return null;

    return (
      <Column noMargin>
        <Line>
          <Column noMargin>
            <Text>
              This will export your game as a Cordova project. Cordova is a
              technology that enables HTML5 games to be packaged for <b>iOS</b>,{' '}
              <b>Android</b> and more.
            </Text>
            <Text>
              Third-party tools like <b>Adobe PhoneGap Build</b> allow game
              developers to bundle their games using Cordova.
            </Text>
          </Column>
        </Line>
        <Line>
          <LocalFolderPicker
            type="export"
            value={this.state.outputDir}
            defaultPath={project.getLastCompilationDirectory()}
            onChange={value => this.setState({ outputDir: value })}
            fullWidth
          />
        </Line>
        <Line>
          <Spacer expand />
          <RaisedButton
            label={<Trans>Export</Trans>}
            primary={true}
            onClick={this.launchExport}
            disabled={!this.state.outputDir}
          />
        </Line>
        <Dialog
          title={t('Export finished')}
          actions={[
            <FlatButton
              key="open"
              label={<Trans>Open folder</Trans>}
              primary={true}
              onClick={this.openExportFolder}
            />,
            <FlatButton
              key="close"
              label={<Trans>Close</Trans>}
              primary={false}
              onClick={() =>
                this.setState({
                  exportFinishedDialogOpen: false,
                })
              }
            />,
          ]}
          secondaryActions={
            <HelpButton key="help" helpPagePath="/publishing" />
          }
          modal
          open={this.state.exportFinishedDialogOpen}
        >
          <Text>
            <Trans>
              You can now compress and upload the game to PhoneGap Build which
              will compile it for you to an iOS and Android app.
            </Trans>
          </Text>
          <Text>
            <Trans>
              You can also compile the game by yourself using Cordova
              command-line tool to iOS (XCode is required) or Android (Android
              SDK is required).
            </Trans>
          </Text>
          <RaisedButton
            fullWidth
            primary
            onClick={() => this.openPhoneGapBuild()}
            label={t('Open PhoneGap Build')}
          />
        </Dialog>
      </Column>
    );
  }
}

export default LocalCordovaExport;
