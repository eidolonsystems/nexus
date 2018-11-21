import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HBoxLayout, Padding, VBoxLayout } from '../..';
import { AccountModel } from '.';
import { MenuBar } from './menu_bar';
import { RolePanel } from './role_panel';

interface Properties {

  /** The account's model. */
  model: AccountModel;
}

interface State {
  isLoading: boolean;
  displaySize: DisplaySize;
}

/** Implements the container used to display account information. */
export class AccountPage extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoading: true,
      displaySize: AccountPage.getDisplaySize()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public componentWillMount() {
    this.props.model.load().then(
      () => {
        this.setState({
          isLoading: false
      });
    });
  }

  public componentDidMount() {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  public render(): JSX.Element {
    const header = ((): JSX.Element => {
      switch(this.state.displaySize) {
        case DisplaySize.LARGE:
          return <LargeHeader name={this.props.model.account.name}
            roles={this.props.model.roles}/>;
        case DisplaySize.MEDIUM:
          return <MediumHeader name={this.props.model.account.name}
            roles={this.props.model.roles}/>;
        case DisplaySize.SMALL:
          return <SmallHeader name={this.props.model.account.name}
            roles={this.props.model.roles}/>;
        default:
          return <MediumHeader name={this.props.model.account.name}
            roles={this.props.model.roles}/>;
      }
    })();
    return (
      <VBoxLayout height='100%' width='100%'>
        {header}
      </VBoxLayout>);
  }

  private static getDisplaySize(): DisplaySize {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return DisplaySize.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return DisplaySize.MEDIUM;
    } else {
      return DisplaySize.LARGE;
    }
  }

  private onScreenResize(): void {
    const newDisplaySize = AccountPage.getDisplaySize();
    if(newDisplaySize !== this.state.displaySize) {
      this.setState({displaySize: newDisplaySize});
    }
  }
}

class HeaderUnderline extends React.PureComponent {
  public render(): JSX.Element {
    return <div className={css(HeaderUnderline.STYLE.base)}/>;
  }

  private static STYLE = StyleSheet.create({
    base: {
      width: '100%',
      height: '1px',
      backgroundColor: '#E6E6E6'
    }
  });
}

interface HeaderProps {
  name: string;
  roles: Nexus.AccountRoles;
}

class LargeHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <Padding/>
          <HBoxLayout width='1036px' height='40px'>
            <Padding size='18px'/>
            <MenuBar displaySize={DisplaySize.LARGE}/>
            <Padding/>
            <div className={
                css(LargeHeader.STYLE.usernameAndRoleContainer)}>
              <div className={
                  css(LargeHeader.STYLE.usernameAndRoleWrapper)}>
                <UsernameLabel name={this.props.name} height='40px'/>
                <Padding size='10px'/>
                <RolePanel roles={this.props.roles}/>
              </div>
            </div>
            <Padding size='18px'/>
          </HBoxLayout>
          <Padding/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    usernameAndRoleWrapper: {
      height: '40px',
      width: 'auto',
      display: 'flex'
    },
    usernameAndRoleContainer: {
      height: '40px',
      display: 'flex',
      flexWrap: 'wrap'
    }
  });
}

class MediumHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <div className={css(MediumHeader.STYLE.headerPadding)}/>
          <Padding size='18px'/>
          <HBoxLayout height='40px' width='750px'>
            <MenuBar displaySize={DisplaySize.MEDIUM}/>
            <div className={css(MediumHeader.STYLE.innerPadding)}/>
            <RolePanel roles={this.props.roles}/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div className={css(MediumHeader.STYLE.headerPadding)}/>
        </HBoxLayout>
        <HeaderUnderline/>
        <HBoxLayout width='100%' height='30px'>
          <div className={css(MediumHeader.STYLE.headerPadding)}/>
          <Padding size='18px'/>
          <HBoxLayout height='30px' width='750px'>
            <Padding/>
            <UsernameLabel name={this.props.name} height='30px'/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div className={css(MediumHeader.STYLE.headerPadding)}/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    headerPadding: {
      width: 'calc(50% - 393px)',
      height: '100%'
    },
    innerPadding: {
      width: 'calc(100% - 658px)',
      height: '100%'
    }
  });
}

class SmallHeader extends React.Component<HeaderProps> {
  public render(): JSX.Element {
    return (
      <VBoxLayout width='100%'>
        <HBoxLayout width='100%' height='40px'>
          <div className={css(SmallHeader.STYLE.headerPadding)}/>
          <Padding size='18px'/>
          <HBoxLayout height='40px' className={
              css(SmallHeader.STYLE.accountHeader)}>
            <MenuBar displaySize={DisplaySize.SMALL}/>
            <div className={css(SmallHeader.STYLE.innerPadding)}/>
            <RolePanel roles={this.props.roles}/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div className={css(SmallHeader.STYLE.headerPadding)}/>
        </HBoxLayout>
        <HeaderUnderline/>
        <HBoxLayout width='100%' height='30px'>
          <div className={css(SmallHeader.STYLE.headerPadding)}/>
          <Padding size='18px'/>
          <HBoxLayout height='30px' className={
              css(SmallHeader.STYLE.accountHeader)}>
            <Padding/>
            <UsernameLabel name={this.props.name} height='30px'/>
          </HBoxLayout>
          <Padding size='18px'/>
          <div className={css(SmallHeader.STYLE.headerPadding)}/>
        </HBoxLayout>
        <HeaderUnderline/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    accountHeader: {
      width: '60%',
      minWidth: '284px',
      maxWidth: '424px'
    },
    headerPadding: {
      width: 'calc(20% - 18px)'
    },
    innerPadding: {
      width: 'calc(45% - 68px)',
      height: '100%'
    }
  });
}

interface UsernameProps {
  name: string;
  height: string;
}

class UsernameLabel extends React.Component<UsernameProps>  {
  public render(): JSX.Element {
    return (
      <VBoxLayout height={this.props.height}>
        <Padding/>
        <span className={css(UsernameLabel.STYLE.username)}>
          {this.props.name}
        </span>
        <Padding/>
      </VBoxLayout>);
  }

  private static STYLE = StyleSheet.create({
    username: {
      font: '500 14px Roboto',
      color: '#4B23A0',
      whiteSpace: 'nowrap'
    }
  });
}
