import { css, StyleSheet } from 'aphrodite/no-important';
import * as Nexus from 'nexus';
import * as React from 'react';
import { Button, DisplaySize, HLine, Modal } from '../../..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** The list of rule schemas that define the new rules. */
  schemas: Nexus.ComplianceRuleSchema[];

  /** Determines if the modal should be open or not. */
  isOpen?: boolean;

  /** The callback to hide or show the uploader. */
  onToggleModal?: () => void;

  /** The callback to add a rule with the following schema. */
  onAddNewRule?: (newRule: Nexus.ComplianceRuleSchema) => void;
}

interface State {
  selection: number;
}

/** Displays a component that allows a user to add a new rule. */
export class NewRuleModal extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    isOpen: false,
    onToggleModal: () => {},
    onAddNewRule: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      selection: -1
    }
  }

  public render(): JSX.Element {
    const modalVisibility = (() => {
      if(this.props.isOpen) {
        return null;
      } else {
        return NewRuleModal.STYLE.hidden;
      }
    })();
    const plusSignImageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return '20px';
      } else {
        return '16px';
      }
    })();
    const buttonWrapper = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return NewRuleModal.STYLE.buttonWrapperSmall;
      } else {
        return NewRuleModal.STYLE.buttonWrapper;
      }
    })();
    const options = [];
    for(let i = 0; i < this.props.schemas.length; ++i) {
      if(i === this.state.selection) {
        options.push(
          <div className={css(NewRuleModal.EXTRA_STYLE.selectedRow)}
              onClick={this.onClickRule.bind(this, i)}>
            {this.props.schemas[i].name}
          </div>);
      } else {
        options.push(
          <div className={css(NewRuleModal.EXTRA_STYLE.optionRow)}
            onClick={this.onClickRule.bind(this, i)}>
            {this.props.schemas[i].name}
          </div>);
      }
    }
    return (
      <div>
        <div style={NewRuleModal.STYLE.newRuleRow}
            onClick={this.props.onToggleModal}>
          <div style={NewRuleModal.STYLE.imageWrapper}>
            <img src={NewRuleModal.ADD_PATH}
              height={plusSignImageSize}
              width={plusSignImageSize}/>
          </div>
          <div style={NewRuleModal.STYLE.newRuleText}>{'Add New Rule'}</div>
        </div>
        <div style={modalVisibility}>
          <Modal displaySize={this.props.displaySize}
              height='461px' width='300px'>
            <div style={NewRuleModal.STYLE.header}>
              <div style={NewRuleModal.STYLE.headerText}>
                Add New Rule
              </div>
              <img height={plusSignImageSize} width={plusSignImageSize}
                onClick={this.props.onToggleModal}
                src={NewRuleModal.CLOSE_PATH}/>
            </div>
            <div style={NewRuleModal.STYLE.ruleItemWrapper}>
              {options}
            </div>
            <div style={NewRuleModal.STYLE.footerWrapper}>
              <HLine color='#E6E6E6'/>
              <div style={buttonWrapper}>
                <Button label='Select'
                  onClick={this.addNewRule.bind(this)}/>
              </div>
            </div>
          </Modal>
        </div>
      </div>);
  }

  private onClickRule(index: number) {
    if(index === this.state.selection) {
      this.setState({selection: -1});
    } else {
      this.setState({selection: index});
    }
  }

  private addNewRule() {
    if(this.state.selection === -1) {
      return;
    }
    this.props.onAddNewRule(this.props.schemas[this.state.selection]);
    this.props.onToggleModal();
    this.setState({selection: -1});
  }

  private static readonly STYLE = {
    newRuleRow: {
      height: '20px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      cursor: 'pointer' as 'pointer',
      width: '130px'
    },
    newRuleText: {
      font: '400 14px Roboto',
      paddingLeft: '18px'
    },
    imageWrapper: {
      display: 'flex' as 'flex',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      width: '20px',
      height: '20px',
      cursor: 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      height: '38px',
      paddingLeft: '18px',
      paddingRight: '18px',
      paddingTop: '18px'
    },
    footerWrapper: {
      boxSizing: 'border-box' as 'border-box',
      paddingLeft: '18px',
      paddingRight: '18px'
    },
    headerText: {
      font: '400 16px Roboto',
      height: '20px',
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px'
    },
    iconWrapperLarge: {
      height: '16px',
      width: '16px'
    },
    ruleItemWrapper: {
      paddingTop: '30px',
      paddingBottom: '30px'
    },
    buttonWrapperSmall: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      paddingTop: '30px',
      paddingBottom: '40px'
    },
    buttonWrapper: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center',
      paddingTop: '30px',
      paddingBottom: '18px'
    }
  };

  private static readonly EXTRA_STYLE = StyleSheet.create({
    optionRow: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      font: '400 14px Roboto',
      color: '#000000',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      paddingLeft: '18px',
      ':hover': {
        backgroundColor: '#F8F8F8'
      }
    },
    selectedRow: {
      boxSizing: 'border-box' as 'border-box',
      height: '40px',
      width: '100%',
      font: '400 14px Roboto',
      color: '#FFFFFF',
      backgroundColor: '#684BC7',
      display: 'flex' as 'flex',
      alignItems: 'center' as 'center',
      paddingLeft: '18px'
    }
  });
  private static readonly CLOSE_PATH =
    'resources/account_page/compliance_page/new_rule_modal/close.svg';
  private static readonly ADD_PATH = 
  'resources/account_page/compliance_page/new_rule_modal/add.svg';
}
