import {
  AdministrationClient,
  DirectoryEntry,
  CurrencyId,
  Money
} from 'spire-client';
import preloaderTimer from 'utils/preloader-timer';
import userService from 'services/user';

class Controller {
  constructor(componentModel) {
    this.componentModel = clone(componentModel);
    this.componentModel.directoryEntry = new DirectoryEntry(
      this.componentModel.directoryEntry.id,
      this.componentModel.directoryEntry.type,
      this.componentModel.directoryEntry.name
    );
    this.adminClient = new AdministrationClient();
  }

  getView() {
    return this.view;
  }

  setView(view) {
    this.view = view;
  }

  /** @private */
  getRequiredData() {
    let directoryEntry = this.componentModel.directoryEntry;
    let loadAccountRiskParameters = this.adminClient.loadRiskParameters.apply(this.adminClient, [directoryEntry]);
    let loadAccountRoles = this.adminClient.loadAccountRoles.apply(this.adminClient, [directoryEntry]);

    return Promise.all([
      loadAccountRiskParameters,
      loadAccountRoles
    ]);
  }

  componentDidMount() {
    let directoryEntry = this.componentModel.directoryEntry;
    let requiredDataFetchPromise = this.getRequiredData();

    preloaderTimer.start(
      requiredDataFetchPromise,
      null,
      Config.WHOLE_PAGE_PRELOADER_WIDTH,
      Config.WHOLE_PAGE_PRELOADER_HEIGHT
    ).then((responses) => {
      this.componentModel.riskParameters = responses[0];
      this.componentModel.directoryEntry = directoryEntry;
      this.componentModel.roles = responses[1];
      this.componentModel.userName = directoryEntry.name;
      this.componentModel.isAdmin = userService.isAdmin();
      this.view.update(this.componentModel);
    });
  }

  isModelInitialized() {
    let model = clone(this.componentModel);
    delete model.componentId;
    delete model.directoryEntry;
    return !$.isEmptyObject(model);
  }

  onCurrencyChange(newCurrencyNumber) {
    EventBus.publish(Event.Profile.RiskControls.CURRENCY_SELECTED);
    this.componentModel.riskParameters.currencyId = CurrencyId.fromNumber(newCurrencyNumber);
    this.view.update(this.componentModel);
  }

  onNetLossChange(newAmount) {
    this.componentModel.riskParameters.netLoss = Money.fromValue(newAmount);
  }

  onBuyingPowerChange(newAmount) {
    this.componentModel.riskParameters.buyingPower = Money.fromValue(newAmount);
  }

  onTransitionTimeChange(newTime) {
    this.componentModel.riskParameters.transitionTime = newTime;
  }

  save() {
    if (this.componentModel.riskParameters.currency != 0) {
      let riskParameters = this.componentModel.riskParameters;
      let directoryEntry = this.componentModel.directoryEntry;
      this.adminClient.storeRiskParameters.apply(this.adminClient, [directoryEntry, riskParameters])
        .then(onSaved.bind(this))
        .catch(onFailed.bind(this));
    } else {
      this.view.showSaveFailedMessage('Currency not selected');
    }

    function onSaved() {
      this.view.showSavedMessage('Saved');
    }

    function onFailed() {
      this.view.showSaveFailedMessage('Failed');
    }
  }
}

export default Controller;
