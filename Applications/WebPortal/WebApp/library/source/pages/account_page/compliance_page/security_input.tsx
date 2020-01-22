import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';
import { DisplaySize, HLine, Modal } from '../../..';

interface Properties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The list of securities to display. */
  value?: Nexus.ComplianceValue[];

  /** Called when the list of values changes.
   * @param value - The updated list.
   */
  onChange?: (value: Nexus.ComplianceValue[]) => void;
}

interface State {
  isEditing: boolean;
  localValue: Nexus.ComplianceValue[];
  selection: number;
  inputString: string;
}

/** A component that displays a list of securities. */
export class SecurityInput extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: '',
    readonly: false,
    onChange: () => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      isEditing: false,
      localValue: this.props.value.slice(),
      selection: -1,
      inputString: ''
    }
    this.toggleEditing = this.toggleEditing.bind(this);
    this.selectEntry = this.selectEntry.bind(this);
    this.onInputChange = this.onInputChange.bind(this);
    this.addEntry = this.addEntry.bind(this);
    this.removeEntry = this.removeEntry.bind(this);
    this.onSubmitChange = this.onSubmitChange.bind(this);
  }

  public render() {
    const visibility = (() => {
      if(!this.state.isEditing) {
        return SecurityInput.STYLE.hidden;
      } else {
        return null;
      }
    })();
    const findSymbolBox = (() => {
      if(this.props.readonly) {
        return SecurityInput.STYLE.hidden;
      } else {
        return SecurityInput.STYLE.findSymbolBox;
      }
    })();
    const headerText = (() => {
      if(this.props.readonly) {
        return SecurityInput.MODAL_HEADER_READONLY;
      } else {
        return SecurityInput.MODAL_HEADER;
      }
    })();
    const iconRowStyle = (() => {
      if(this.props.readonly) {
        return SecurityInput.STYLE.hidden;
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return SecurityInput.STYLE.iconRowSmall;
        } else {
          return SecurityInput.STYLE.iconRowBig;
        }
      }
    })();
    const imageSize = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return SecurityInput.IMAGE_SIZE_SMALL_VIEWPORT;
      } else {
        return SecurityInput.IMAGE_SIZE_LARGE_VIEWPORT;
      }
    })();
    const iconWrapperStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.readonly) {
          return SecurityInput.STYLE.iconWrapperSmallReadonly;
        } else {
          return SecurityInput.STYLE.iconWrapperSmall;
        }
      } else {
        if(this.props.readonly) {
          return SecurityInput.STYLE.iconWrapperLargeReadonly;
        } else {
          return SecurityInput.STYLE.iconWrapperLarge;
        }
      }
    })();
    const uploadButton = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return (
          <div style={iconWrapperStyle}>
            <img height={imageSize} width={imageSize}
              src={SecurityInput.PATH + 'upload-grey.svg'}/>
          </div>);
      } else {
        return (
          <div style={{...iconWrapperStyle}}>
            <img height={imageSize} width={imageSize}
              src={SecurityInput.PATH + 'upload-grey.svg'}/>
            <div style={SecurityInput.STYLE.iconLabelReadonly}>
              {SecurityInput.UPLOAD_TEXT}
            </div>
          </div>);
      }
    })();
    const removeButton = (() => {
      if(this.state.selection !== -1) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry.bind(this)}>
              <img height={imageSize} width={imageSize}
                style={SecurityInput.STYLE.iconClickableStyle}
                src={SecurityInput.PATH + 'remove-purple.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry.bind(this)}>
              <img height={imageSize} width={imageSize}
                style={SecurityInput.STYLE.iconClickableStyle}
                src={SecurityInput.PATH + 'remove-purple.svg'}/>
              <div style={SecurityInput.STYLE.iconLabel}>
                {SecurityInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return (
            <div style={iconWrapperStyle}>
              <img height={imageSize} width={imageSize}
                src={SecurityInput.PATH + 'remove-grey.svg'}/>
            </div>);
        } else {
          return (
            <div style={iconWrapperStyle}
                onClick={this.removeEntry.bind(this)}>
              <img height={imageSize} width={imageSize}
                src={SecurityInput.PATH + 'remove-grey.svg'}/>
              <div style={SecurityInput.STYLE.iconLabelReadonly}>
                {SecurityInput.REMOVE_TEXT}
              </div>
            </div>);
        }
      }
    })();
    const confirmationButton = (() => {
      if(this.props.readonly) {
        return (
          <button className={css(SecurityInput.EXTRA_STYLE.button)}
              onClick={this.toggleEditing}>
            {SecurityInput.CONFIRM_TEXT}
          </button>);
      } else {
        return (
          <button className={css(SecurityInput.EXTRA_STYLE.button)}
            onClick={this.onSubmitChange}>
            {SecurityInput.SUBMIT_CHANGES_TEXT}
          </button>);
      }
    })();
    let displayValue  = '';
    for (let i =0; i < this.props.value.length; ++i) {
      const sec = this.props.value[i].value as Nexus.Security;
      displayValue = displayValue.concat(sec.symbol.toString());
      if(i >= 0 && i < this.props.value.length - 1 && 
          this.props.value.length > 1) {
        displayValue = displayValue.concat(', ');
      }
    }
    return(
      <div>
        <input
          style={SecurityInput.STYLE.textBox}
          className={css(SecurityInput.EXTRA_STYLE.effects)}
          value={displayValue}
          onFocus={this.toggleEditing.bind(this)}
          onClick={this.toggleEditing.bind(this)}/>
        <div style={visibility}>
          <Modal displaySize={this.props.displaySize} 
              width={'300px'} height={'559px'}
              onClose={this.toggleEditing}>
            <div style={SecurityInput.STYLE.header}>
              <div style={SecurityInput.STYLE.headerText}>
                {headerText}
              </div>
              <img src={SecurityInput.PATH + 'close.svg'}
                height='20px'
                width='20px'
                style={SecurityInput.STYLE.clickable}
                onClick={this.toggleEditing}/>
            </div>
            <input
              className={css(SecurityInput.EXTRA_STYLE.effects)}
              style={findSymbolBox}
              placeholder={SecurityInput.PLACEHOLDER_TEXT}
              onChange={this.onInputChange}
              onKeyDown={this.addEntry}
              value={this.state.inputString}/>
            <SymbolsBox 
              displaySize={this.props.displaySize}
              readonly={this.props.readonly}
              value={this.state.localValue}
              selection={this.state.selection}
              onClick={this.selectEntry}/>
            <div style={iconRowStyle}>
              {removeButton}
              {uploadButton}
            </div>
            <HLine color={'#e6e6e6'}/>
            <div style={SecurityInput.STYLE.buttonWrapper}>
              {confirmationButton}
            </div>
          </Modal>
        </div>
      </div>);
  }

  private toggleEditing(){
    this.setState({
      isEditing: !this.state.isEditing,
      selection: -1,
      localValue: this.props.value.slice()
    });
  }

  private selectEntry(index: number) {
    this.setState({selection: index});
  }

  private removeEntry() {
    if(this.state.selection !== -1) {
      this.setState({localValue: 
        this.state.localValue.slice(0, this.state.selection).concat(
        this.state.localValue.slice(this.state.selection+1))});
    }
    this.setState({selection: -1});
  }

  private onInputChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.setState({inputString: event.target.value});
  }

  private onSubmitChange(){
    this.props.onChange(this.state.localValue);
    this.toggleEditing();
  }

  private addEntry(event: React.KeyboardEvent<HTMLInputElement>) {
    if(event.keyCode === 13) {
      const newParameter = 
        new Nexus.ComplianceValue( 
          Nexus.ComplianceValue.Type.SECURITY, 
          new Nexus.Security(
            this.state.inputString,
            Nexus.MarketCode.NONE,
            Nexus.DefaultCountries.CA));
      this.setState({localValue: 
        this.state.localValue.slice().concat(newParameter)});
      this.setState({inputString: ''});
    }
  }

  private static readonly STYLE = {
    textBox: {
      textOverflow: 'ellipsis' as 'ellipsis',
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      width: '100%',
      paddingLeft: '10px',
      cursor: 'pointer' as 'pointer'
    },
    hidden: {
      visibility: 'hidden' as 'hidden',
      display: 'none' as 'none'
    },
    clickable: {
      cursor: 'pointer' as 'pointer'
    },
    header: {
      boxSizing: 'border-box' as 'border-box',
      width: '100%',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-between' as 'space-between',
      height: '20px',
      marginBottom: '30px'
    },
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default' as 'default'
    },
    findSymbolBox: {
      width: '100%',
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
    },
    iconClickableStyle: {
      cursor: 'pointer' as 'pointer'
    },
    iconWrapperSmall: {
      height: '24px',
      width: '24px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'pointer' as 'pointer'
    },
    iconWrapperSmallReadonly: {
      height: '24px',
      width: '24px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'default' as 'default'
    },
    iconWrapperLarge:  {
      height: '16px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'pointer' as 'pointer'
    },
    iconWrapperLargeReadonly:  {
      height: '16px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      justifyContent: 'center' as 'center',
      pointer: 'default' as 'default'
    },
    iconLabel: {
      color: '#333333',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'pointer' as 'pointer'
    },
    iconLabelReadonly: {
      color: '#C8C8C8',
      font: '400 14px Roboto',
      paddingLeft: '8px',
      cursor: 'default' as 'default'
    },
    iconRowSmall: {
      height: '24px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly' 
    },
    iconRowBig: {
      height: '16px',
      width: '100%',
      marginBottom: '30px',
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      justifyContent: 'space-evenly' as 'space-evenly' 
    },
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '1px solid #684BC7',
      borderRadius: '1px',
      font: '400 16px Roboto'
    },
    buttonWrapper: {
      marginTop: '30px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'wrap' as 'wrap',
      justifyContent: 'center' as 'center',
      alignItems: 'center' as 'center'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      }
    },
    button: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '246px',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      border: '0px solid #684BC7',
      borderRadius: '1px',
      font: '400 16px Roboto',
      outline: 'none',
      MozAppearance: 'none' as 'none',
      cursor: 'pointer' as 'pointer',
      ':active' : {
        backgroundColor: '#4B23A0'
      },
      ':focus': {
        border: 0,
        outline: 'none',
        borderColor: '#4B23A0',
        backgroundColor: '#4B23A0',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none',
        MozAppearance: 'none' as 'none'
      },
      ':hover':{
        backgroundColor: '#4B23A0'
      },
      '::-moz-focus-inner': {
        border: 0,
        outline: 0
      },
      ':-moz-focusring': {
        outline: 0
      }
    }
  });
  private static readonly MODAL_HEADER = 'Edit Symbols';
  private static readonly MODAL_HEADER_READONLY = 'Find symbol here';
  private static readonly PLACEHOLDER_TEXT = 'Find symbol here';
  private static readonly SUBMIT_CHANGES_TEXT = 'Submit Changes';
  private static readonly CONFIRM_TEXT = 'OK';
  private static readonly UPLOAD_TEXT = 'Upload';
  private static readonly REMOVE_TEXT = 'Remove';
  private static readonly PATH =
    'resources/account_page/compliance_page/security_input/';
  private static readonly IMAGE_SIZE_SMALL_VIEWPORT = '20px';
  private static readonly IMAGE_SIZE_LARGE_VIEWPORT = '16px';
}

interface InputFieldProperties {
}

export class InputField extends React.Component<SymbolsBoxProperties> {

}
interface SymbolsBoxProperties {

  /** The size at which the component should be displayed at. */
  displaySize: DisplaySize;

  /** Determines if the component is readonly. */
  readonly?: boolean;

  /** The currently selected value. */
  selection: number;

  /** The list of securities to display. */
  value: Nexus.ComplianceValue[];

  /** Called when the list of values changes.
   * @param value - The updated list.
   */
  onClick?: (index: number) => void;
}

export class SymbolsBox extends React.Component<SymbolsBoxProperties> {
  public render() {
    const scrollHeader = (() => {
      if(!this.props.readonly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return(
            <div style={SymbolsBox.STYLE.scrollBoxHeaderSmall}>
              {'Added Symbols'}
            </div>);
        } else {
          return(
            <div style={SymbolsBox.STYLE.scrollBoxHeaderLarge}>
              {'Added Symbols'}
            </div>);
        }
      } else {
        return null;
      }
    })();
    const selectedSecuritiesBox = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.props.readonly) {
          return SymbolsBox.STYLE.scrollBoxSmallReadonly;
        } else {
          return SymbolsBox.STYLE.scrollBoxSmall;
        }
      } else {
        if(this.props.readonly) {
          return SymbolsBox.STYLE.scrollBoxBigReadonly;
        } else {
          return SymbolsBox.STYLE.scrollBoxBig;
        }
      }
    })();
    const entries = [];
    for(let i = 0; i < this.props.value.length; ++i) {
        const sec = this.props.value[i].value;
        if(this.props.readonly) {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntryReadonly}>
              {sec.symbol.toString()}
            </div>);
        } else if(this.props.selection === i) {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntrySelected}
                onClick={this.selectEntry.bind(this, i)}>
              {sec.symbol.toString()}
            </div>);
        } else {
          entries.push(
            <div style={SymbolsBox.STYLE.scrollBoxEntry}
                onClick={this.selectEntry.bind(this, i)}>
              {sec.symbol.toString()}
            </div>);
        }
    }
    return (
      <div style={selectedSecuritiesBox}>
        {scrollHeader}
        {entries}
      </div>);
  }

  private selectEntry(index: number) {
    if(!this.props.readonly) {
      if(index === this.props.selection) {
        this.props.onClick(-1);
      } else {
        this.props.onClick(index);
      }
    }
  }

  private static readonly STYLE = {
    headerText: {
      font: '400 16px Roboto',
      flexGrow: 1,
      cursor: 'default' as 'default'
    },
    scrollBoxSmall: {
      boxSizing: 'border-box' as 'border-box',
      height: '246px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxSmallReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '342px',
      width: '246px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxBig: {
      boxSizing: 'border-box' as 'border-box',
      height: '280px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxBigReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '342px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      overflowY: 'auto' as 'auto'
    },
    scrollBoxHeaderSmall: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '246px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky' as 'sticky',
      top: 0,
      cursor: 'default' as 'default'
    },
    scrollBoxHeaderLarge: {
      boxSizing: 'border-box' as 'border-box',
      backgroundColor: '#FFFFFF',
      height: '40px',
      maxWidth: '264px',
      color: '#4B23A0',
      font: '500 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      borderBottom: '1px solid #C8C8C8',
      position: 'sticky' as 'sticky',
      top: 0,
      cursor: 'default' as 'default'
    },
    scrollBoxEntry: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    },
    scrollBoxEntryReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#FFFFFF',
      color: '#000000',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      cursor: 'default' as 'default'
    },
    scrollBoxEntrySelected: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      width: '100%',
      backgroundColor: '#684BC7',
      color: '#FFFFFF',
      font: '400 14px Roboto',
      paddingLeft: '10px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      alignItems: 'center' as 'center',
      cursor: 'pointer' as 'pointer'
    }
  };
}
