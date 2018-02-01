import './style.scss';
import React from 'react';
import PrimaryButton from 'components/reusables/common/primary-button';
import ProfileSearchPanel from 'components/reusables/common/profile-search-panel';
import CommonView from 'components/structures/common/accounts/search/common-view';

class View extends CommonView {
  constructor(react, controller, componentModel) {
    super(react, controller, componentModel);
  }

  render() {
    let newAccountBtnModel = {
      label: 'New Account'
    };

    let newGroupModel = {
      label: 'New Group'
    };

    let onNewAccountClick = this.onNewAccountClick.bind(this);
    let onNewGroupClick = this.onNewGroupClick.bind(this);
    let onSearchInputChange = this.onSearchInputChange.bind(this);
    let loadGroupAccounts = this.controller.loadGroupAccounts.bind(this.controller);

    let navigateToTraderProfile = this.controller.navigateToTraderProfile.bind(this.controller);
    let navigateToGroupProfile = this.controller.navigateToGroupProfile.bind(this.controller);
    let panels = [];
    if (this.componentModel.groupedAccounts != null) {
      for (let i=0; i<this.componentModel.groupedAccounts.length; i++) {
        let panelModel = {
          groupId: this.componentModel.groupedAccounts[i].id,
          groupName: this.componentModel.groupedAccounts[i].name,
          accounts: this.componentModel.groupedAccounts[i].accounts || null,
          isLoaded: this.componentModel.groupedAccounts[i].isLoaded,
          searchString: this.componentModel.searchString
        };
        panels.push(
          <ProfileSearchPanel key={i}
                              model={panelModel}
                              navigateToTraderProfile={navigateToTraderProfile}
                              navigateToGroupProfile={navigateToGroupProfile}
                              loadAccounts={loadGroupAccounts}/>
        );
      }
    }

    let newGroupCreateBtnModel = {
      label: 'Create'
    };

    let newGroupCancelBtnModel = {
      label: 'Cancel'
    };

    return (
      <div id="search-profiles-container">
        <div className="search-profiles-wrapper">
          <div className="menu-info-wrapper">
            <PrimaryButton className="new-account-btn" model={newAccountBtnModel} onClick={onNewAccountClick}/>
            <PrimaryButton className="new-group-btn" model={newGroupModel} onClick={onNewGroupClick}/>
          </div>
          <div className="search-wrapper">
            <div className="search-input-wrapper">
              <input className="search-input" onChange={onSearchInputChange}/>
              <span className="icon-magnifying-glass"></span>
            </div>
            <div className="search-results-wrapper">
              {panels}
            </div>
          </div>

          <div id="new-group-modal" className="modal fade" tabIndex="-1" role="dialog">
            <div className="modal-dialog" role="document">
              <div className="modal-content">
                <div className="modal-header">
                  Create Group
                  <span className="icon-close" onClick={this.onNewGroupCancelClick}></span>
                </div>
                <div className="modal-body">
                  <input type="text" defaultValue="" className="name-input" onKeyPress={this.onGroupNamePress.bind(this)}/>
                  <div className="divider"></div>
                  <div className='buttons-wrapper'>
                    <PrimaryButton model={newGroupCreateBtnModel}
                                   className="create-button single-button"
                                   onClick={this.onNewGroupCreateClick.bind(this)}/>
                  </div>
                </div>
                <div className="modal-bottom"></div>
              </div>
            </div>
          </div>
        </div>
      </div>
    );
  }
}

export default View;
