import * as Beam from 'beam';
import * as React from 'react';
import * as ReactDOM from 'react-dom';
import { DisplaySize, displaySizeRenderer, GroupPage, GroupSubPage }
  from 'web_portal';

interface Properties {
  displaySize: DisplaySize;
}

interface State {
  subPage: GroupSubPage;
  group: Beam.DirectoryEntry;
}

class GroupPageTester extends React.Component<Properties, State> {
  constructor(props: Properties) {
    super(props);
    this.state = {
      subPage: GroupSubPage.NONE,
      group: Beam.DirectoryEntry.makeAccount(124, 'Group_name_goes_here')
    };
  }

  public render(): JSX.Element {
    return (
      <GroupPage displaySize={this.props.displaySize}
        subPage={this.state.subPage} group={this.state.group}
        onMenuClick={this.onMenuClick}/>);
  }

  private onMenuClick = (subPage: GroupSubPage) => {
    this.setState({subPage: subPage});
  }
}

const ResponsiveGroupPageTester = displaySizeRenderer(GroupPageTester);

ReactDOM.render(<ResponsiveGroupPageTester/>, document.getElementById('main'));