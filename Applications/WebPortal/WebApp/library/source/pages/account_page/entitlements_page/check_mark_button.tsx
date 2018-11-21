import * as React from 'react';

/** The properties used to display the BurgerButton. */
interface Properties {

  /** The width and height of the button. */
  size: number | string;

  /** The onClick event handler. */
  onClick?(event?: React.MouseEvent<any>): void;

  /** Determines if the checkmark is currenly active(green) or inactive(grey) */
  isChecked: boolean;
}

/** Displays a check mark button. */
export class CheckMarkButton extends React.Component<Properties, {}> {
  static readonly defaultProps = {
    onClick: () => {}
  }

  constructor(properties: Properties) {
    super(properties);
  }

  public render(): JSX.Element {
    const source = (() => {
      if(this.props.isChecked) {
        return (
          'resources/account_page/entitlements_page/icons/check-green.svg');
      } else {
        return (
          'resources/account_page/entitlements_page/icons/check-grey.svg');
      }
    })();
    return (
      <img src={source}
        alt='checkbox'
        height={this.props.size}
        width={this.props.size}
        onClick={this.props.onClick}
        style={CheckMarkButton.STYLE.base}/>);
  }

  private static STYLE = {
    base: {
      cursor: 'pointer'
    }
  };
}
