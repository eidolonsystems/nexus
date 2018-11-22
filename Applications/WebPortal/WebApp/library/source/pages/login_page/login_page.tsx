import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';
import { Center, HBoxLayout, Padding, VBoxLayout } from '../../';

interface Properties {

  /** The state of the login operation. */
  status: LoginPage.Status;

  /** The error message to display. */
  errorMessage?: string;

  /** Indicates the user has submitted the login credentials.
   * @param username - The account's username.
   * @param password - The account's password.
   */
  onSubmit?: (username: string, password: string) => void;
}

/** Displays the Login Page. */
export class LoginPage extends React.Component<Properties> {
  public static readonly defaultProps = {
    errorMessage: '',
    onSubmit: (_u: string, _p: string) => {}
  }

  constructor(properties: Properties) {
    super(properties);
    this.onSubmit = this.onSubmit.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onUsernameChange = this.onUsernameChange.bind(this);
  }

  public render(): JSX.Element {
    const [staticStyle, animatedStyle] = (() => {
      switch(this.props.status) {
        case LoginPage.Status.NONE:
          return [css(LoginPage.DYNAMIC_STYLES.logoVisible),
            css(LoginPage.DYNAMIC_STYLES.logoInvisible)];
        case LoginPage.Status.LOADING:
          return [css(LoginPage.DYNAMIC_STYLES.logoInvisible),
            css(LoginPage.DYNAMIC_STYLES.logoVisible)];
      }
    })();
    return (
      <Center width='100%' height='100%' style={LoginPage.STATIC_STYLES.page}>
        <HBoxLayout width='320px' height='462px'>
          <Padding size='18px'/>
          <VBoxLayout width='284px' height='462px'>
            <Padding size='60px'/>
            <HBoxLayout width='100%' height='50px'>
              <Padding/>
              <object data='resources/login_page/logo-static.svg'
                type='image/svg+xml' className={staticStyle} tabIndex={-1}/>
              <object data='resources/login_page/logo-animated.svg'
                type='image/svg+xml' className={animatedStyle} tabIndex={-1}/>
              <Padding/>
            </HBoxLayout>
            <Padding size='60px'/>
            <input type='text' placeholder='Username' autoComplete='off'
              className={css(LoginPage.DYNAMIC_STYLES.inputBox)}
              onFocus={() => this.usernameInputField.placeholder = ''}
              onBlur={() => this.usernameInputField.placeholder = 'Username'}
              onChange={this.onUsernameChange}
              ref={(ref) => this.usernameInputField = ref}/>
            <Padding size='20px'/>
            <input type='password' placeholder='Password' autoComplete='off'
              className={css(LoginPage.DYNAMIC_STYLES.inputBox)}
              onFocus={() => this.passwordInputField.placeholder = ''}
              onBlur={() => this.passwordInputField.placeholder = 'Password'}
              ref={(ref) => this.passwordInputField = ref}/>
            <Padding size='50px'/>
            <button className={css(LoginPage.DYNAMIC_STYLES.signInButton)}
                disabled={this.props.status === LoginPage.Status.LOADING}
                onClick={this.onSubmit} ref={(ref) => this.submitButton = ref}>
              Sign In
            </button>
            <Padding size='30px'/>
            <span style={LoginPage.STATIC_STYLES.errorMessage}>
              {this.props.errorMessage}
            </span>
            <Padding size='60px'/>
          </VBoxLayout>
          <Padding size='18px'/>
        </HBoxLayout>
      </Center>);
  }

  public componentDidMount(): void {
    window.addEventListener('keydown', this.onKeyDown);
    this.submitButton.disabled = true;
  }

  public componentWillUnmount(): void {
    window.removeEventListener('keydown', this.onKeyDown);
  }

  private onSubmit() {
    this.props.onSubmit(this.usernameInputField.value,
      this.passwordInputField.value);
  }

  private onKeyDown(event: KeyboardEvent) {
    if(document.activeElement !== this.usernameInputField &&
        document.activeElement !== this.passwordInputField &&
        document.activeElement !== this.submitButton &&
        event.key.trim().length === 1) {
      this.usernameInputField.focus();
    } else if((document.activeElement === this.submitButton ||
        document.activeElement === this.passwordInputField) &&
        event.key.trim() === 'Enter') {
      this.onSubmit();
    }
  }

  private onUsernameChange() {
    this.submitButton.disabled = this.usernameInputField.value.trim() === '';
  }

  private static readonly STATIC_STYLES = {
    page: {
      backgroundColor: '#4B23A0'
    },
    logo: {
      width: '100%',
      height: '100%'
    },
    errorMessage: {
      width: '100%',
      textAlign: 'center' as 'center',
      font: '300 14px Roboto',
      height: '20px',
      color: '#FAEB96'
    }
  }

  private static DYNAMIC_STYLES = StyleSheet.create({
    inputBox: {
      width: '284px',
      padding: 0,
      height: '34px',
      borderColor: '#FFFFFF',
      backgroundColor: '#4B23A0',
      borderWidth: '0px 0px 1px 0px',
      color: '#FFFFFF',
      font: '300 16px Roboto',
      outline: 0,
      textAlign: 'center',
      borderRadius: 0,
      '::placeholder': {
        color: '#FFFFFF'
      },
      '::-moz-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-input-placeholder': {
        color: '#FFFFFF',
        opacity: 1
      },
      '::-ms-clear': {
        display: 'none'
      },
      '::-ms-reveal': {
        display: 'none'
      },
      '::-webkit-autofill': {
        backgroundColor:  'none'
      },
      '::-webkit-credentials-auto-fill-button': {
        visibility: 'hidden' as 'hidden',
        display: 'none !important',
        pointerEvents: 'none',
        height: 0,
        width: 0,
        margin: 0
      }
    },
    logoVisible: {
      width: '130px',
      height: '50px'
    },
    logoInvisible: {
      width: '0px',
      height: '0px'
    },
    signInButton: {
      width: '284px',
      height: '48px',
      color: '#4B23A0',
      backgroundColor: '#E2E0FF',
      font: '400 20px Roboto',
      borderRadius: '1px',
      border: 'none',
      outline: 0,
      ':hover': {
        backgroundColor: '#FFFFFF'
      },
      '::-moz-focus-inner': {
        border: 0
      },
      ':disabled': {
        backgroundColor: '#684BC7',
        border: 'none'
      },
      ':focus': {
        ':not(:disabled)': {
          border: '1px solid white'
        }
      }
    }
  });
  private usernameInputField: HTMLInputElement;
  private passwordInputField: HTMLInputElement;
  private submitButton: HTMLButtonElement;
}

export namespace LoginPage {

  /** The state of the login operation. */
  export enum Status {

    /** No login operation is being performed. */
    NONE,

    /** The login operation is pending. */
    LOADING
  }
}
