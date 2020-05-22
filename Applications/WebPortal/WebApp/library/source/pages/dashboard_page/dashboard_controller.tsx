import * as Beam from 'beam';
import * as React from 'react';
import * as Router from 'react-router-dom';
import * as Path from 'path-to-regexp';
import { DisplaySize, LoadingPage } from '../..';
import { AccountController, AccountDirectoryController } from '..';
import { DashboardModel } from './dashboard_model';
import { DashboardPage } from './dashboard_page';
import { SideMenu } from './side_menu';

interface Properties {

  /** The model to use. */
  model: DashboardModel;

  /** The device's display size. */
  displaySize: DisplaySize;

  /** The URL prefix that navigates to this page. */
  urlPrefix?: string;

  /** Indicates the user has logged out. */
  onLogout?: () => void;
}

interface State {
  isLoaded: boolean;
  redirect: string;
}

/** Implements the controller for the DashboardPage. */
export class DashboardController extends React.Component<Properties, State> {
  private static readonly defaultProps = {
    urlPrefix: ''
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isLoaded: false,
      redirect: null
    };
    this.onSideMenuClick = this.onSideMenuClick.bind(this);
  }

  public render(): JSX.Element {
    if(this.state.redirect) {
      return <Router.Redirect push to={this.state.redirect}/>;
    }
    if(!this.state.isLoaded) {
      return <LoadingPage/>;
    }
    return (
      <DashboardPage roles={this.props.model.roles}
          onSideMenuClick={this.onSideMenuClick}>
        <Router.Switch>
          <Router.Route path={`${this.props.urlPrefix}/account`}
            render={this.renderAccountPage}/>
          <Router.Route path={`${this.props.urlPrefix}/account_directory`}
            render={() =>
              <AccountDirectoryController
                displaySize={this.props.displaySize}
                roles={this.props.model.roles}
                countryDatabase={this.props.model.countryDatabase}
                model={this.props.model.accountDirectoryModel}/>}/>
          <Router.Route>
            <Router.Redirect to={`${this.props.urlPrefix}/account`}/>
          </Router.Route>
        </Router.Switch>
      </DashboardPage>);
  }

  public componentDidMount(): void {
    this.props.model.load().then(
      () => {
        this.setState({isLoaded: true});
      });
  }

  public componentDidUpdate(): void {
    if(this.state.redirect) {
      this.setState({redirect: null});
    }
  }

  private renderAccountPage = () => {
    const model = (() => {
      const pattern = Path.pathToRegexp(
        `${this.props.urlPrefix}/account/:id(\\d+)?`, [], { end: false });
      const match = pattern.exec(window.location.pathname);
      const account = (() => {
        if(match[1]) {
          return Beam.DirectoryEntry.makeAccount(parseInt(match[1]), '');
        }
        return this.props.model.account;
      })();
      return this.props.model.makeAccountModel(account);
    })();
    return (
      <AccountController
        entitlements={this.props.model.entitlementDatabase}
        countryDatabase={this.props.model.countryDatabase}
        currencyDatabase={this.props.model.currencyDatabase}
        marketDatabase={this.props.model.marketDatabase}
        model={model}
        displaySize={this.props.displaySize}/>);
  }

  private onSideMenuClick(item: SideMenu.Item) {
    if(item === SideMenu.Item.PROFILE) {
      this.setState({redirect: `${this.props.urlPrefix}/account`});
    } else if(item === SideMenu.Item.ACCOUNTS) {
      this.setState({redirect: `${this.props.urlPrefix}/account_directory`});
    } else if(item === SideMenu.Item.SIGN_OUT) {
      this.props.model.logout().then(this.props.onLogout);
    }
  }
}
