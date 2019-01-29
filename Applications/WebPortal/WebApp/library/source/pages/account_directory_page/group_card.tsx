import { css, StyleSheet } from 'aphrodite/no-important';
import * as Beam from 'beam';
import { VBoxLayout, Padding } from 'dali';
import * as React from 'react';
import { Transition } from 'react-transition-group';
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
}

interface State {
  oldHeight: number;
}

/** A card that displays a group and the accounts associated with it. */
export class GroupCard extends React.Component<Properties, State> {
  constructor(properties: Properties) {
    super(properties);
    this.state = {
      oldHeight: 0
    };
    this.onClick = this.onClick.bind(this);
  }

  public render(): JSX.Element {
    const headerStyle = (() => {
      if(this.props.isOpen) {
        return GroupCard.STYLE.textOpen;
      } else {
        return GroupCard.STYLE.text;
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
    const accounts = (() => {
      const accountDetails = [];
      if(this.props.accounts.length > 0) {
        for(let i = 0; i < this.props.accounts.length; ++i) {
          accountDetails.push(
            <div style={GroupCard.STYLE.accountBox}
                key={this.props.accounts[i].account.id}>
              <div style={{...accountsLableStyle,
                  ...GroupCard.STYLE.accountLabelText}}>
                {this.props.accounts[i].account.name.toString()}
              </div>
              <RolePanel roles={this.props.accounts[i].roles}/>
            </div>);
        }
      } else {
        accountDetails.push(
          <div style={{...accountsLableStyle,
              ...GroupCard.STYLE.emptyLableText}}>
            Empty
          </div>);
      }
      return accountDetails;
    })();
    const maxContainerHeight = (() => {
      if(this.props.isOpen && this.ANIMATION_STYLE.entered.maxHeight !== `${16 + 32 + (32 * this.props.accounts.length)}px`) {
        this.ANIMATION_STYLE.entered.maxHeight = `${16 + 32 + (32 * this.props.accounts.length)}px`;
        this.ANIMATION_STYLE.entering.maxHeight = `${16 + 32 + (32 * this.props.accounts.length)}px`;
      }
      return 5;
    })();
    return (
      <VBoxLayout width='100%'>
        <div style={GroupCard.STYLE.header}>
          <DropDownButton size='16px'
            onClick={this.onClick}
            isExpanded={this.props.isOpen}/>
          <div style={headerStyle}>{this.props.group.name}</div>
        </div>
        <Transition in={this.props.isOpen}
            timeout={GroupCard.TRANSITION_LENGTH_MS}>
          {(state) => (
            <div style={(this.ANIMATION_STYLE as any)[state]}>
              <VBoxLayout width='100%'>
                <HLine color='#E6E6E6'/>
                <Padding size='10px'/>
                {accounts}
                <Padding size='20px'/>
              </VBoxLayout>
            </div>)}
        </Transition>
      </VBoxLayout>);
  }

  public componentDidMount(): void {
    console.log('Is open on create? : ' +  this.props.isOpen);
  }

  private onClick(): void {
    this.props.onClick(this.props.group);
  }

  private static readonly STYLE = {
    box: {
      width: '100%'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      marginLeft: '10px',
      marginRight: '10px'
    },
    headerSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      marginLeft: '10px',
      marginRight: '10px'
    },
    textOpen: {
      marginLeft: '18px',
      font: '500 14px Roboto',
      color: '#4B23A0'
    },
    text: {
      marginLeft: '18px',
      font: '400 14px Roboto',
      color: '#000000'
    },
    accountLabelSmall: {
      marginLeft: '10px'
    },
    accountLabelMedium: {
      marginLeft: '34px'
    },
    accountLabelLarge: {
      marginLeft: '34px'
    },
    accountLabelText: {
      font: '400 14px Roboto',
      color: '#000000',
      marginRight: '10px'
    },
    emptyLableText: {
      font: '400 14px Roboto',
      color: '#8C8C8C'
    },
    accountBox: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between' as 'space-between',
      marginRight: '10px'
    }
  };
  private ANIMATION_STYLE = {
    entering: {
      maxHeight: '0',
      transitionProperty: 'max-height, transform',
      transform: 'scaleY(1)',
      transitionDuration: `${GroupCard.TRANSITION_LENGTH_MS}ms`,
      overflow: 'hidden' as 'hidden',
      transformOrigin: 'top' as 'top'
    },
    entered: {
      maxHeight: '0',
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
