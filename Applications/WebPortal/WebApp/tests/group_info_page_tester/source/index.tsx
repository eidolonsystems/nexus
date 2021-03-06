import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Router from 'react-router-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  accounts: WebPortal.AccountEntry[];
}

/** Displays a sample GroupInfoPage for testing. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      accounts: []
    };
  }

  public render(): JSX.Element {
    return(
      <Router.BrowserRouter>
        <Router.Route path="/"
          component={
            Router.withRouter(() => 
              <WebPortal.GroupInfoController
                accounts={this.state.accounts}
                displaySize={this.props.displaySize}/>
            )}>
        </Router.Route>
        <div style={TestApp.STYLE.testingComponents}>
          <button onClick={this.clearAccounts}>No Members</button>
          <button onClick={this.addSomeAccounts}>Some Members</button>
          <button onClick={this.addManyAccounts}>Lots of Members</button>
        </div>
      </Router.BrowserRouter>);
  }

  public componentDidMount() {
    this.addManyAccounts();
  }

  private clearAccounts = () => {
    this.setState({accounts: []});
  }

  private addSomeAccounts = () => {
    let group = [];
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(12, 'Bob'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(18, 'Sue'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(13, 'Carl'),
      new Nexus.AccountRoles(2)));
    this.setState({accounts: group});
  }

  private addManyAccounts = () => {
    let group = [];
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(127, 'Bob'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(118, 'Sue'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(913, 'Carl'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(912, 'Max'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(138, 'Dean'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(3, 'Froddo'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(12, 'Sam'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(18, 'Merry'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(13, 'Pippin'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(312, 'Spire'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(218, 'Nexus'),
      new Nexus.AccountRoles(2)));
    group.push(
      new WebPortal.AccountEntry(Beam.DirectoryEntry.makeAccount(113, 'Doug'), 
      new Nexus.AccountRoles(2)));
    this.setState({accounts: group});
  }

  private static STYLE = {
    testingComponents: {
      position: 'fixed' as 'fixed',
      fontSize: '8px',
      top: 0,
      left: 0,
      zIndex: 500
    }
  };
}

const ResponsivePage = WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
