import * as ReactDOM from 'react-dom';
import * as React from 'react';
import * as WebPortal from 'web_portal';

interface State {
  displaySize: WebPortal.DisplaySize;
}

/**  Displays a testing application. */
class TestApp extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props);
    this.state = {
      displaySize: TestApp.getDisplaySize()
    };
    this.onScreenResize = this.onScreenResize.bind(this);
  }

  public render(): JSX.Element {
    return (
    <WebPortal.HBoxLayout>
      <WebPortal.Padding size='12px'/>
        <WebPortal.VBoxLayout>
          <WebPortal.Padding size='20px'/>
          <WebPortal.ProfilePage displaySize={this.state.displaySize}/>
        </WebPortal.VBoxLayout>
      <WebPortal.Padding/>
    </WebPortal.HBoxLayout>
    );
  }

  public componentDidMount(): void {
    window.addEventListener('resize', this.onScreenResize);
  }

  public componentWillUnmount(): void {
    window.removeEventListener('resize', this.onScreenResize);
  }

  private static getDisplaySize(): WebPortal.DisplaySize {
    const screenWidth = window.innerWidth ||
      document.documentElement.clientWidth ||
      document.getElementsByTagName('body')[0].clientWidth;
    if(screenWidth <= 767) {
      return WebPortal.DisplaySize.SMALL;
    } else if(screenWidth > 767 && screenWidth <= 1035) {
      return WebPortal.DisplaySize.MEDIUM;
    } else {
      return WebPortal.DisplaySize.LARGE;
    }
  }

  private onScreenResize() {
    const newDisplaySize = TestApp.getDisplaySize();
    if(newDisplaySize !== this.state.displaySize) {
      this.setState({ displaySize: newDisplaySize });
    }
    console.log(this.state.displaySize.toString());
  }
}

ReactDOM.render(<TestApp/>, document.getElementById('main'));

