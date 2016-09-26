import userService from 'services/user';
import {browserHistory} from 'react-router/es6';
import store from 'store';
import LocalstorageKey from 'commons/localstorage-keys';

class Controller {
  constructor(react) {
    this.componentModel = {
      isAdmin: false,
      isManager: false
    };
  }

  /** @private */
  onMenuOpened() {
    this.view.openMenu();
  }

  /** @private */
  onSignedIn() {
    this.componentModel = {
      isAdmin: userService.isAdmin(),
      isManager: userService.isManager()
    };
    this.view.update(this.componentModel);
  }

  /** @private */
  saveLastVisitedPath(path) {
    store.set(LocalstorageKey.LAST_VISITED_PATH, path);
  }

  /** @private */
  onCloseMenu() {
    this.view.closeMenu.apply(this.view);
  }

  getView() {
    return this.view;
  }

  setView(aView) {
    this.view = aView;
  }

  componentDidMount() {
    this.view.initialize();
    this.menuOpenedEventListenerId = EventBus.subscribe(Event.TopNav.SIDE_MENU_OPENED, this.onMenuOpened.bind(this));
    this.signedInEventListenerId = EventBus.subscribe(Event.Application.SIGNED_IN, this.onSignedIn.bind(this));
    this.closeMenuEventListenerId = EventBus.subscribe(Event.TopNav.CLOSE_SIDE_MENU, this.onCloseMenu.bind(this));
  }

  componentWillUnmount() {
    this.view.dispose();
    EventBus.unsubscribe(this.menuOpenedEventListenerId);
    EventBus.unsubscribe(this.signedInEventListenerId);
    EventBus.unsubscribe(this.closeMenuEventListenerId);
  }

  signOut() {
    userService.signOut();
    browserHistory.push('/');
  }

  navigateToProfileAccount() {
    let userDirectoryEntry = userService.getDirectoryEntry();
    let path = '/profile-account' +
      '/' + userDirectoryEntry.type +
      '/' + userDirectoryEntry.id +
      '/' + userDirectoryEntry.name;
    let eventHandlerId = EventBus.subscribe(Event.Application.BLANK_PAGE_LOADED, () => {
      EventBus.unsubscribe(eventHandlerId);
      browserHistory.replace(path);
      this.saveLastVisitedPath(path);
    });
    browserHistory.push('blank');
  }

  navigateToSearchProfiles() {
    let path = '/searchProfiles-search';
    browserHistory.push(path);
    this.saveLastVisitedPath(path);
  }

  publishSideMenuClosed() {
    EventBus.publish(Event.TopNav.SIDE_MENU_CLOSED);
  }
}

export default Controller;
