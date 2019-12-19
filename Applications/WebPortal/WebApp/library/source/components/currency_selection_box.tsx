import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The set of available currencies to select. */
  currencyDatabase: Nexus.CurrencyDatabase;

  /** The initial currency selected. */
  value?: Nexus.Currency;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the currency selection box. */
  className?: string;

  /** The event handler called when the selection changes. */
  onChange?: (currency: Nexus.Currency) => void;
}

/** Displays a selection box for currencies. */
export class CurrencySelectionBox extends React.Component<Properties> {
  public static readonly defaultProps = {
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const currencies = [];
    for(let currency of this.props.currencyDatabase) {
      currencies.push(
        <option value={currency.code} key={currency.code}>
          {currency.code}
        </option>);
    }
    const defaultValue = (() => {
      if(this.props.value) {
        return this.props.currencyDatabase.fromCurrency(this.props.value).code;
      }
      return undefined;
    })();
    return (
      <div>
        <select 
            style={{...CurrencySelectionBox.STYLE.selectionBoxStyle,
              ...this.props.style}} 
            className={css(CurrencySelectionBox.EXTRA_STYLE.noDefaults) + ' ' +
              this.props.className}
            onChange={this.onChange} defaultValue={defaultValue}>
          {currencies}
        </select>
      </div>);
  }

  private onChange(event: React.ChangeEvent<any>): void {
    if(this.props.onChange) {
      this.props.onChange(this.props.currencyDatabase.fromCode(
        event.target.value).currency);
    }
  }

  private static readonly STYLE = {
    selectionBoxStyle: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      font: '400 14px Roboto',
      paddingLeft: '7px',
      color: '#000000',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      backgroundColor: '#F2F2F2',
      backgroundImage:
        'url(resources/account_page/profile_page/arrow-down.svg)',
      backgroundRepeat: 'no-repeat',
      backgroundPosition: 'right 10px top 50%',
      backgroundSize: '8px 6px',
      MozAppearance: 'none' as 'none',
      WebkitAppearance: 'none' as 'none',
      appearance: 'none' as 'none',
      minWidth: '246px',
      width: '100%',
      flexGrow: 1,
      flexShrink: 1
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    noDefaults: {
      ':focus': {
        ouline: 0,
        outlineColor: 'transparent',
        outlineStyle: 'none',
        border: '1px solid #684BC7',
        borderRadius: '1px'
      },
      '::moz-focus-inner': {
        border: 0
      },
      ':-moz-focusring': {
        color: 'transparent',
        textShadow: '0 0 0 #000'
      },
      '-webkit-user-select': 'text',
      '-moz-user-select': 'text',
      '-ms-user-select': 'text',
      'user-select': 'text'
    }
  });
}
