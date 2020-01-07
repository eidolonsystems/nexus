import { css, StyleSheet } from 'aphrodite';
import * as React from 'react';

interface Properties {

  /** The value to display in the field. */
  value?: number;

  /** Additional CSS styles. */
  style?: any;

  /** The class name of the input field. */
  className?: string;

  /** The smallest number that the box accepts (inclusive). */
  min?: number;

  /** The largest number that the box accepts (inclusive). */
  max?: number;

  /** Determines if the component is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: number) => void;
}

/** Displays a decimal number input field. */
export class NumberInput extends React.Component<Properties> {
  public static readonly defaultProps = {
    value: 0,
    readonly: false,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        return NumberInput.STYLE.boxReadonly;
      } else {
        return NumberInput.STYLE.box;
      }
    })();
    return (
      <input type={'number'}
        min={this.props.min}
        max={this.props.max}
        value={this.props.value}
        style={{...boxStyle, ...this.props.style}}
        disabled={this.props.readonly}
        onChange={this.onChange}
        className={css(NumberInput.EXTRA_STYLE.customHighlighting) + ' ' +
          this.props.className}/>);
  }

  private onChange(event: React.ChangeEvent<HTMLInputElement>) {
    this.props.onChange(event.target.valueAsNumber);
  }

  private static STYLE = {
    box: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      minWidth: '184px',
      maxWidth: '246px',
      width: '100%',
      paddingLeft: '10px',
      WebkitAppearance: 'textfield',
      appearance: 'none' as 'none'
    },
    boxReadonly: {
      boxSizing: 'border-box' as 'border-box',
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      font: '400 14px Roboto',
      color: '#333333',
      flexGrow: 1,
      minWidth: '184px',
      maxWidth: '246px',
      width: '100%',
      paddingLeft: '10px',
      WebkitAppearance: 'textfield',
      appearance: 'none' as 'none',
      backgroundColor: '#FFFFFF'
    }
  };
  private static EXTRA_STYLE = StyleSheet.create({
    customHighlighting: {
      '-moz-appearance': 'textfield',
      ':focus': {
        borderColor: '#684BC7',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::-webkit-inner-spin-button': {
        '-webkit-appearance': 'none',
        'appearance': 'none',
        margin: 0
      },
      '::-webkit-outer-spin-button': {
        '-webkit-appearance': 'none',
        'appearance': 'none',
        margin: 0
      }
    }
  });
}
