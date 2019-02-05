import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import { VBoxLayout } from 'dali';
import * as React from 'react';
import { Transition, TransitionGroup } from 'react-transition-group';
import { DisplaySize, DropDownButton, HLine } from '../..';
import { RolePanel } from '../account_page/role_panel';
import { AccountEntry } from '.';

interface Properties {

  /** Determines the size to display the component at. */
  displaySize: DisplaySize;

  /** The group the card belongs to. */
  group: Beam.DirectoryEntry;

  /** The accounts in the group. */
  accounts: AccountEntry[];

  /** The current filter used on the accounts. */
  filter: string;

  /**  Determines if the card is opened. */
  isOpen: boolean;

  /** Called when the card is clicked. */
  onClick: (group: Beam.DirectoryEntry) => void;

  onAccountClick?: (account: Beam.DirectoryEntry) => void;
}

/** A card that displays a group and the accounts associated with it. */
export class GroupCard extends React.Component<Properties> {
  public render(): JSX.Element {
    const headerTextStyle = (() => {
      if(this.props.isOpen) {
        return GroupCard.STYLE.headerTextOpen;
      } else {
        return GroupCard.STYLE.headerText;
      }
    })();
    const headerStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return GroupCard.DYNAMIC_STYLE.header;
      } else {
        return GroupCard.DYNAMIC_STYLE.header;
      }
    })();
    const accountsLableStyle = (() => {
      switch(this.props.displaySize) {
        case(DisplaySize.SMALL):
          return GroupCard.STYLE.accountLabelSmall;
        case(DisplaySize.MEDIUM):
          return GroupCard.STYLE.accountLabelMedium;
        case(DisplaySize.LARGE):
          return GroupCard.STYLE.accountLabelLarge;
      }
    })();
    const timeout = (() => {
      if(this.props.isOpen) {
        return GroupCard.TRANSITION_LENGTH_MS / 2;
      } else {
        return GroupCard.TRANSITION_LENGTH_MS;
      }
    })();
    const accounts: JSX.Element[] = [];
    if(this.props.accounts.length > 0) {
      for(const account of this.props.accounts) {
        if(account.account.name.indexOf(this.props.filter) === 0
            && this.props.filter) {
          accounts.push(
           <div className={css(GroupCard.DYNAMIC_STYLE.accountBox)}
              key={account.account.id}
              onClick={() => this.props.onAccountClick(account.account)}>
            <div style={{...accountsLableStyle,
                ...GroupCard.STYLE.accountLabelText}}>
              <div style={GroupCard.STYLE.highlightedText}>
                {account.account.name.slice(0, this.props.filter.length)}
              </div>
              {account.account.name.slice(this.props.filter.length)}
            </div>
            <div style={GroupCard.STYLE.rolesBox}>
              <RolePanel roles={account.roles}/>
            </div>
          </div>
          );
        } else {
          accounts.push(
          <Transition in={this.props.isOpen}
            appear={true}
            timeout={timeout}>
          {(state) => (
          <div className={css(GroupCard.DYNAMIC_STYLE.accountBox)}
              style={{...this.accountLabelAnimationStyle.base,
                ...(this.accountLabelAnimationStyle as any)[state]}}
              key={account.account.id}
              onClick={() => this.props.onAccountClick(account.account)}>
            <div style={{...accountsLableStyle,
                ...GroupCard.STYLE.accountLabelText}}>
              {account.account.name.toString()}
            </div>
            <div>{state}</div>
            <div style={GroupCard.STYLE.rolesBox}>
              <RolePanel roles={account.roles}/>
            </div>
          </div>
          )}
          </Transition>);
        }
      }
    } else {
      accounts.push(
         <Transition in={this.props.isOpen}
            timeout={timeout}>
          {(state) => (
          <div key={this.props.group.id} style={{...accountsLableStyle,
              ...GroupCard.STYLE.emptyLableText,
              ...(this.accountLabelAnimationStyle as any)[state]}}>
            Empty
          </div>
          )}
        </Transition>);
    }
    const topAccountPadding = (() => {
      if(this.props.accounts.length === 0) {
        return '14px';
      } else {
        return '10px';
      }
    })();
    return (
      <VBoxLayout width='100%'>
        <div className={css(headerStyle)}>
          <DropDownButton size='16px'
            isExpanded={this.props.isOpen}
            onClick={() => this.props.onClick(this.props.group)}/>
          <div style={headerTextStyle}>{this.props.group.name}</div>
        </div>
        <Transition in={this.props.isOpen}
            timeout={timeout}>
          {(state) => (
            <div>
              <div style={(this.topPaddingAnimationStyle as any)[state]}>
                <HLine color='#E6E6E6'/>
                <div style={{height: topAccountPadding}}/>
              </div>
              {accounts}
              <div style={(this.bottomPaddingAnimationStyle as any)[state]}>
                <div style={{height:'20px'}}/>
              </div>
            </div>)}
        </Transition>
      </VBoxLayout>);
  }

  private static readonly STYLE = {
    box: {
      width: '100%'
    },
    headerTextOpen: {
      marginLeft: '18px',
      font: '500 14px Roboto',
      color: '#4B23A0'
    },
    headerText: {
      marginLeft: '18px',
      font: '400 14px Roboto',
      color: '#000000'
    },
    accountLabelSmall: {
      marginLeft: 0
    },
    accountLabelMedium: {
      marginLeft: '38px'
    },
    accountLabelLarge: {
      marginLeft: '38px'
    },
    accountLabelText: {
      font: '400 14px Roboto',
      color: '#000000',
      flexGrow: 0,
      flexShrink: 0,
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap'
    },
    emptyLableText: {
      font: '400 14px Roboto',
      color: '#8C8C8C',
      paddingLeft: '10px'
    },
    highlightedText: {
      font: '400 14px Roboto',
      color: '#000000',
      backgroundColor: '#FFF7C4',
      flexGrow: 0,
      flexShrink: 0
    },
    rolesBox: {
      width: '80px',
      flexGrow: 0,
      flexShrink: 0
    }
  };
  private static DYNAMIC_STYLE = StyleSheet.create({
    accountBox: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      paddingLeft: '10px',
      paddingRight: '10px',
      ':hover' : {
        backgroundColor: '#F8F8F8'
      }
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      paddingLeft: '10px',
      paddingRight: '10px',
      cursor: 'pointer' as 'pointer',
      ':hover' : {
        backgroundColor: '#F8F8F8'
      }
    }
  });
  private accountLabelAnimationStyle = {
    base: {
      transitionProperty: 'max-height, transform',
      transform: 'scaleY(1)',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    entering: {
      maxHeight: 0,
      transitionProperty: 'max-height, transform',
      transform: 'scaleY(1)',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    entered: {
      maxHeight: '34px',
      transform: 'scaleY(1)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    }
  };
  private topPaddingAnimationStyle = {
    entering: {
      maxHeight: 0,
      transitionProperty: 'max-height, transform',
      transform: 'scaleY(1)',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    entered: {
      maxHeight: '15px',
      transform: 'scaleY(1)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    }
  };
  private bottomPaddingAnimationStyle = {
    entering: {
      maxHeight: 0,
      transitionProperty: 'max-height, transform',
      transform: 'scaleY(1)',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    entered: {
      maxHeight: '20px',
      transform: 'scaleY(1)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exiting: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      transitionProperty: 'max-height, transform',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    exited: {
      maxHeight: 0,
      transform: 'scaleY(0)',
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    }
  };
  private static readonly TRANSITION_LENGTH_MS = 200;
}
