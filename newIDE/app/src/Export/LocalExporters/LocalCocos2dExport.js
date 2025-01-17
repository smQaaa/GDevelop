// @flow
import { Trans } from '@lingui/macro';
import React, { Component } from 'react';
import Dialog from '../../UI/Dialog';
import FlatButton from '../../UI/FlatButton';
import RaisedButton from '../../UI/RaisedButton';
import Toggle from '../../UI/Toggle';
import { sendExportLaunched } from '../../Utils/Analytics/EventSender';
import { Column, Line, Spacer } from '../../UI/Grid';
import HelpButton from '../../UI/HelpButton';
import { showErrorBox } from '../../UI/Messages/MessageBox';
import { findGDJS } from './LocalGDJSFinder';
import localFileSystem from './LocalFileSystem';
import LocalFolderPicker from '../../UI/LocalFolderPicker';
import assignIn from 'lodash/assignIn';
import optionalRequire from '../../Utils/OptionalRequire';
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
  debugMode: boolean,
|};

export default class LocalCocos2dExport extends Component<Props, State> {
  state = {
    exportFinishedDialogOpen: false,
    outputDir: '',
    debugMode: false,
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
    const { project } = this.props;
    if (!project) return;

    sendExportLaunched('local-cocos2d');

    const { outputDir, debugMode } = this.state;
    project.setLastCompilationDirectory(outputDir);

    LocalCocos2dExport.prepareExporter()
      .then(({ exporter }) => {
        exporter.exportWholeCocos2dProject(project, debugMode, outputDir);
        exporter.delete();
        this.setState({
          exportFinishedDialogOpen: true,
        });
      })
      .catch(err => {
        showErrorBox('Unable to export the game with Cocos2d-JS', err);
      });
  };

  openExportFolder = () => {
    if (shell) shell.openItem(this.state.outputDir);
  };

  render() {
    const { project } = this.props;
    if (!project) return null;

    return (
      <Column noMargin>
        <Line>
          <Text>
            <Trans>
              This will export your game using Cocos2d-JS game engine. The game
              can be compiled for Android or iOS if you install Cocos2d-JS
              developer tools.
            </Trans>
          </Text>
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
          <Toggle
            onToggle={(e, check) =>
              this.setState({
                debugMode: check,
              })
            }
            toggled={this.state.debugMode}
            labelPosition="right"
            label={
              <Trans>
                Debug mode (show FPS counter and stats in the bottom left)
              </Trans>
            }
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
          title={<Trans>Export finished</Trans>}
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
            You can now upload the game to a web hosting or use Cocos2d-JS
            command line tools to export it to other platforms like iOS (XCode
            is required) or Android (Android SDK is required).
          </Text>
        </Dialog>
      </Column>
    );
  }
}
