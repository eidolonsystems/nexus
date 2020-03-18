import { css, StyleSheet } from 'aphrodite';
import * as Nexus from 'nexus';
import * as React from 'react';

interface Properties {

  /** The current value of the input field. */
  value: string;

  /** Called when the displayed value changes.
   * @param value - The new value.
   */
  onChange: (value: string) => void;

  /** Called when the value is submitted.
   * @param value - The compliance value that is being submitted.
   */
  onEnter: (value: Nexus.Security) => void;
}

/** The field that allows the user to add a new entry to the list. */
export class SecurityInput extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onInputChange = this.onInputChange.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
  }

  public render() {
    return (<input
      className={css(SecurityInput.EXTRA_STYLE.effects)}
      style={SecurityInput.STYLE.input}
      placeholder={SecurityInput.PLACEHOLDER_TEXT}
      onChange={this.onInputChange}
      onKeyDown={this.onKeyDown}
      value={this.props.value} />);
  }

  private onInputChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.props.onChange(event.target.value);
  }

  private onKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if (event.keyCode === 13) {
      const newSecurity =
        new Nexus.Security(
          this.props.value,
          Nexus.MarketCode.NONE,
          Nexus.DefaultCountries.CA);
      this.props.onEnter(newSecurity);
    }
  }

  private static readonly STYLE = {
    input: {
      width: '100%',
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      height: '34px',
      paddingLeft: '10px',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      marginBottom: '18px'
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
    }
  });

  private static readonly PLACEHOLDER_TEXT = 'Find symbol here';
}
