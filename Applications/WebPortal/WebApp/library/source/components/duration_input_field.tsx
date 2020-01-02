import { css, StyleSheet } from 'aphrodite';
import * as Beam from 'beam';
import * as React from 'react';
import { DisplaySize } from '../display_size';
import { IntegerInputBox } from './integer_input_box';

enum TimeUnit {
  HOURS,
  MINUTES,
  SECONDS
}

interface Properties {

  /** The size to display the component at. */
  displaySize: DisplaySize;

  /** The value to display in the field. */
  value?: Beam.Duration;

  /** The largest value the hours field can hold. */
  maxHourValue?: number;

  /** The smallest value the hours field can hold. */
  minHourValue?: number;

  /** Whether the selection box is read only. */
  readonly?: boolean;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onChange?: (value: Beam.Duration) => void;
}

interface State {
  isInFocus: boolean,
  componentWidth: number
}

/** A component that displays and lets a user edit a duration. */
export class DurationInputField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    value: new Beam.Duration(0),
    minHourValue: 0,
    maxHourValue: 99,
    readonly: false,
    onChange: () => {}
  };

  constructor(props: Properties) {
    super(props);
    this.state = {
      isInFocus: false,
      componentWidth: 0
    }
    this.handleResize = this.handleResize.bind(this);
    this.onBlur = this.onBlur.bind(this);
    this.onFocus = this.onFocus.bind(this);
  }

  public render(): JSX.Element {
    const splitTime = this.props.value.split();
    const wrapperStyle = (() => {
      if(this.props.readonly) {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return DurationInputField.STYLE.wrapperSmallReadonly;
        } else {
          return DurationInputField.STYLE.wrapperLargeReadonly;
        }
      } else {
        if(this.props.displaySize === DisplaySize.SMALL) {
          return DurationInputField.STYLE.wrapperSmall;
        } else {
          return DurationInputField.STYLE.wrapperLarge;
        }
      }
    })();
    const focusClassName = (() => {
      if(this.state.isInFocus) {
        return DurationInputField.STYLE.focused;
      } else {
        return null;
      }
    })();
    const hintText = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        if(this.state.componentWidth >= 165) {
          return 'Hr : Min : Sec';
        } else if(this.state.componentWidth >= 141){
          return 'H : M : S';
        } else {
          return '';
        }
      } else {
        return 'Hr : Min : Sec';
      }
    })();
    return (
      <div style={{...wrapperStyle, ...focusClassName}}
          ref={(input) => { this.reference = input; }}
          onFocus={this.onFocus}
          onBlur={this.onBlur}>
        <div style={DurationInputField.STYLE.inner}>
          <IntegerInputBox
            min={this.props.minHourValue} max={this.props.maxHourValue}
            value={splitTime.hours}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={DurationInputField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.HOURS)}
            readonly={this.props.readonly}
            padding={2}/>
          <div style={DurationInputField.STYLE.colon}>{':'}</div>
          <IntegerInputBox
            min={0} max={59}
            value={splitTime.minutes}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={DurationInputField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.MINUTES)}
            readonly={this.props.readonly}
            padding={2}/>
          <div style={DurationInputField.STYLE.colon}>{':'}</div>
          <IntegerInputBox
            min={0} max={59}
            value={splitTime.seconds}
            className={css(DurationInputField.EXTRA_STYLE.effects)}
            style={DurationInputField.STYLE.integerBox}
            onChange={this.onChange.bind(this, TimeUnit.SECONDS)}
            readonly={this.props.readonly}
            padding={2}/>
        </div>
        <div style={DurationInputField.STYLE.placeholder}>
          {hintText}
        </div>
      </div>);
  }
  
  private handleResize() {
    if(this.props.displaySize === DisplaySize.SMALL) {
      if(this.state.componentWidth !== this.reference.clientWidth) {
        this.setState({componentWidth: this.reference.clientWidth});
      }
    }
  }

  public componentDidMount() {
    window.addEventListener('resize', this.handleResize);
    this.handleResize();
  }

  private onFocus() {
    if(!this.props.readonly) {
      this.setState({isInFocus: true})
    }
  }

  private onBlur() {
    if(!this.props.readonly) {
      this.setState({isInFocus: false})
    }
  }

  private onChange(timeUnit: TimeUnit, value: number) {
    const oldDuration = this.props.value.split();
    const newValue = (() => {
      switch(timeUnit) {
        case TimeUnit.HOURS:
          return Beam.Duration.HOUR.multiply(value).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.MINUTES:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(value)).add(
            Beam.Duration.SECOND.multiply(oldDuration.seconds));
        case TimeUnit.SECONDS:
          return Beam.Duration.HOUR.multiply(oldDuration.hours).add(
            Beam.Duration.MINUTE.multiply(oldDuration.minutes)).add(
            Beam.Duration.SECOND.multiply(value));
      }
    })();
    this.props.onChange(newValue);
  }

  private static readonly STYLE = {
    wrapperSmall: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '110px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperLarge: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #C8C8C8',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperSmallReadonly: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      minWidth: '110px',
      width: '100%',
      flexShrink: 1,
      flexGrow: 1,
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #ffffff',
      borderRadius: '1px',
      height: '34px'
    },
    wrapperLargeReadonly: {
      boxSizing: 'border-box' as 'border-box',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      flexShrink: 1,
      maxWidth: '246px',
      backgroundColor: '#ffffff',
      justifyContent: 'space-between' as 'space-between',
      border: '1px solid #ffffff',
      borderRadius: '1px',
      height: '34px'
    },
    inner: {
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexGrow: 1,
      justifyContent: 'flex-start' as 'flex-start',
      alignItems: 'center',
      marginLeft: '9px' 
    },
    integerBox: {
      boxSizing: 'border-box' as 'border-box',
      font: '400 14px Roboto',
      width: '18px',
      height: '17px',
      border: '0px solid #ffffff',
      padding: 0
    },
    colon: {
      width: '10px',
      height: '16px',
      flexGrow: 0,
      flexShrink: 0,
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
    },
    placeholder: {
      font: '500 11px Roboto',
      color: '#8C8C8C',
      display: 'flex' as 'flex',
      justifyContent: 'center',
      alignItems: 'center',
      marginRight: '10px'
    },
    focused: {
      ouline: 0,
      outlineColor: 'transparent',
      outlineStyle: 'none',
      border: '1px solid #684BC7',
      borderRadius: '1px'
    }
  };
  private static readonly EXTRA_STYLE = StyleSheet.create({
    effects: {
      ':focus': {
        ouline: 0,
        border: '0px solid #000000',
        boxShadow: 'none',
        webkitBoxShadow: 'none',
        outlineColor: 'transparent',
        outlineStyle: 'none'
      },
      ':active' : {
        borderColor: '#684BC7'
      },
      '::moz-focus-inner': {
        border: 0
      },
      '::placeholder': {
        color: '#8C8C8C'
      }
    }
  });
  private reference: HTMLDivElement;
}
