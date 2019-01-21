import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as WebPortal from 'web_portal';

const ResponsivePage =
  WebPortal.displaySizeRenderer(WebPortal.AccountDirectoryPage);
ReactDOM.render(<ResponsivePage/>, document.getElementById('main'));
