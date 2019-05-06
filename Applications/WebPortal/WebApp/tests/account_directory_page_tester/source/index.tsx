import * as Beam from 'beam';
import * as Nexus from 'nexus';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

interface Properties {
  displaySize: WebPortal.DisplaySize;
}

interface State {
  roles: Nexus.AccountRoles;
  groups: Beam.Set<Beam.DirectoryEntry>;
  accounts: Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>;
  model: WebPortal.AccountDirectoryModel;
}

/**  Displays and tests the AccountDirectoryPage. */
class TestApp extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      roles: new Nexus.AccountRoles(),
      groups : new Beam.Set<Beam.DirectoryEntry>(),
      accounts: new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>(),
      model: new WebPortal.LocalAccountDirectoryModel(
        new Beam.Set<Beam.DirectoryEntry>(),
        new Beam.Map<Beam.DirectoryEntry, WebPortal.AccountEntry[]>())
    };
    this.changeRole = this.changeRole.bind(this);
  }

  public render(): JSX.Element {
    return (
      <div>
      <WebPortal.AccountDirectoryPage
        displaySize={this.props.displaySize}
        model={this.state.model}
        roles={this.state.roles}/>
        <div style={TestApp.STYLE.testingComponents}>
          <button tabIndex={-1}
              onClick={() =>
                this.changeRole(Nexus.AccountRoles.Role.ADMINISTRATOR)}>
            ADMINISTRATOR
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.TRADER)}>
            TRADER
          </button>
          <button tabIndex={-1}
              onClick={() => this.changeRole(Nexus.AccountRoles.Role.MANAGER)}>
            MANAGER
          </button>
        </div>
      </div>
    );
  }

  public componentDidMount() {
    this.testAdmin.set(Nexus.AccountRoles.Role.ADMINISTRATOR);
    this.testTrader.set(Nexus.AccountRoles.Role.TRADER);
    this.testManager.set(Nexus.AccountRoles.Role.MANAGER);
    const group1 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 80, 'Nexus');
    const group2 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 41, 'Spire');
    const group3 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 31, 'Office');
    const group4 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 36, 'Shire');
    const group5 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 33, 'Mordor');
    const group6 =
      new Beam.DirectoryEntry(Beam.DirectoryEntry.Type.DIRECTORY, 37, 'Bree');
    this.state.groups.add(group1);
    this.state.groups.add(group2);
    this.state.groups.add(group3);
    this.state.groups.add(group4);
    this.state.groups.add(group5);
    this.state.groups.add(group6);
    const accountEntry1 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 9123, 'administration_service'),
      new Nexus.AccountRoles());
    const accountEntry2 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 23, 'daily_service'),
      new Nexus.AccountRoles());
    const accountEntry3 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 223, 'market_data_relay_service'),
      new Nexus.AccountRoles(3));
    const accountEntry4 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 45, 'data_relay_service'),
      new Nexus.AccountRoles());
    const accountEntry5 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 788, 'execution_service'),
      new Nexus.AccountRoles(5));
    const accountEntry6 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 1, 'data_news_service'),
      new Nexus.AccountRoles());
    const accountEntry7 = new WebPortal.AccountEntry(
      new Beam.DirectoryEntry(
        Beam.DirectoryEntry.Type.ACCOUNT, 5, 'data_relay_news'),
      new Nexus.AccountRoles());
    const testArray = [];
    testArray.push(accountEntry1);
    testArray.push(accountEntry2);
    testArray.push(accountEntry3);
    testArray.push(accountEntry4);
    testArray.push(accountEntry5);
    testArray.push(accountEntry6);
    testArray.push(accountEntry7);
    for(const group of this.state.groups) {
      if(group.id % 2 === 0) {
        this.state.accounts.set(group, testArray);
      } else {
        this.state.accounts.set(group, []);
      }
    }
    const testModel = new WebPortal.LocalAccountDirectoryModel(
      this.state.groups, this.state.accounts);
    testModel.load();
    const newModel = new WebPortal.CachedAccountDirectoryModel(testModel);
    this.setState({model: newModel});
  }

  private changeRole(newRole: Nexus.AccountRoles.Role): void {
    if(newRole === Nexus.AccountRoles.Role.ADMINISTRATOR) {
      this.setState({ roles: this.testAdmin });
    }
    if(newRole === Nexus.AccountRoles.Role.TRADER) {
      this.setState({ roles: this.testTrader });
    }
    if(newRole === Nexus.AccountRoles.Role.MANAGER) {
      this.setState({ roles: this.testManager });
    }
  }
  private testAdmin = new Nexus.AccountRoles();
  private testTrader = new Nexus.AccountRoles();
  private testManager = new Nexus.AccountRoles();
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

const ResponsivePage =
  WebPortal.displaySizeRenderer(TestApp);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
