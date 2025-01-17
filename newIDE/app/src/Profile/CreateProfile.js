// @flow
import { Trans } from '@lingui/macro';

import * as React from 'react';
import FlatButton from '../UI/FlatButton';
import RaisedButton from '../UI/RaisedButton';
import { Column, Line, Spacer } from '../UI/Grid';
import Text from '../UI/Text';
import BackgroundText from '../UI/BackgroundText';

type Props = {
  message?: React.Node,
  onLogin: () => void,
  onCreateAccount: () => void,
};

export default ({ message, onLogin, onCreateAccount }: Props) => (
  <Column noMargin>
    <Line>
      <Text>
        {message || (
          <Trans>
            You are not connected. Create an account and connect to access to
            GDevelop online services and build your game for Android, Windows,
            macOS and Linux in one click!
          </Trans>
        )}
      </Text>
    </Line>
    <Line justifyContent="center" alignItems="baseline">
      <RaisedButton
        label={<Trans>Create my account</Trans>}
        onClick={onCreateAccount}
        primary
      />
      <Spacer />
      <Spacer />
      <BackgroundText>or</BackgroundText>
      <Spacer />
      <FlatButton label={<Trans>Login</Trans>} onClick={onLogin} />
    </Line>
  </Column>
);
