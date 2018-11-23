import { css, StyleSheet } from 'aphrodite/no-important';
import * as React from 'react';
import { DisplaySize } from '../../../';
import { HBoxLayout } from '../../../layouts';

enum Effects {
  NONE,
  FOCUSED,
  HOVER
}

interface Properties {

  /** Indicates the input field can not be interacted with. */
  disabled?: boolean;

  /** The value to display in the field. */
  value?: string;

  displaySize: DisplaySize;

  /** Called when the value changes.
   * @param value - The updated value.
   */
  onInput?: (value: string) => void;

}

interface State {
  effects: Effects;
}

/** Displays a single text input field. */
export class TextField extends React.Component<Properties, State> {
  public static readonly defaultProps = {
    disabled: false,
    value: '',
    onInput: (_: string) => {}
  }

  constructor(props: Properties) {
    super(props);
    this.state = {
      effects: Effects.NONE
    };
    this.onInputBlurred = this.onInputBlurred.bind(this);
    this.onInputFocused = this.onInputFocused.bind(this);
    this.onMouseEntered = this.onMouseEntered.bind(this);
    this.onMouseLeft = this.onMouseLeft.bind(this);
  }

  public render(): JSX.Element {
    const boxStyle = ( () => {
      if(this.state.effects === Effects.HOVER) {
        return TextField.STYLE.hoveredBox;
      } else if(this.state.effects === Effects.FOCUSED){
        return TextField.STYLE.focusedBox;
      } else {
        return TextField.STYLE.box;
      }
    })();
    const textStyle = ( () => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return TextField.STYLE.largerText;
      } else {
        return TextField.STYLE.text;
      }
    })();
    const imageStyle = ( () => {
      if(this.state.effects === Effects.HOVER) {
        return TextField.STYLE.image;
      } else {
        return TextField.STYLE.hidden;
      }
    })();
    return (
    <div className={css(boxStyle)} tabIndex={1}
      onMouseEnter={this.onMouseEntered}
      onMouseLeave={this.onMouseLeft}>
      <input value={this.props.value}
        onFocus={this.onInputFocused}
        onBlur={this.onInputBlurred}
        onChange={(event: React.ChangeEvent<HTMLInputElement>) => {
        this.props.onInput(event.target.value);
      }}
        className={css(textStyle)}/>
      <img src={'resources/account_page/edit.svg'}
        className={css(imageStyle)}/>
    </div>);
  }

  private onInputFocused() {
    if(!this.props.disabled) {
      this.setState({
        effects: Effects.FOCUSED
      });
    }
  }

  private onInputBlurred() {
    this.setState({
      effects: Effects.NONE
    });
  }

  private onMouseEntered() {
    if(!this.props.disabled) {
      if(this.state.effects !== Effects.FOCUSED) {
        this.setState({
          effects: Effects.HOVER
        });
      }
    }
  }

  private onMouseLeft() {
    if(this.state.effects !== Effects.FOCUSED) {
     this.setState({
        effects: Effects.NONE
      });
    }
  }
  private static STYLE = StyleSheet.create({
    box: {
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #FFFFFF'
    },
    hoveredBox: {
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #C8C8C8'
    },
    focusedBox: {
      height: '34px',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      justifyContent: 'space-between',
      border: '1px solid #684BC7'
    },
    image: {
      visibility: 'visible' as 'visible',
      height: '14px',
      width: '14px',
      paddingRight: '10px'
    },
    hidden: {
      opacity: 0,
      height: '14px',
      width: '14px',
      paddingRight: '10px'
    },
    text: {
      font: '400 14px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      paddingLeft: '10px',
      border: '1px solid #FFFFFF'
    },
    largerText: {
      font: '400 16px Roboto',
      color: '#000000',
      whiteSpace: 'nowrap',
      paddingLeft: '10px',
      border: '1px solid #FFFFFF'
    }
  });
}
