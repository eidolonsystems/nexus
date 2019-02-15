import { HBoxLayout, VBoxLayout, Padding } from 'dali';
import { DisplaySize } from '../../..';
import * as React from 'react';
import { ProfilePage } from './profile_page';

interface Properties {

  /** The name of the input field. */
  name: string;

  /** Determines if the input is editable. */
  readonly?: boolean;

  /** Whether the form is rendered vertically or horizontally. */
  displaySize: DisplaySize;

  /** The input field to render. */
  children: JSX.Element;
}

/** Displays an input form for a single item. */
export class FormEntry extends React.Component<Properties> {
  public static readonly defaultProps = {
    readonly: false
  }

  constructor(props: Properties) {
    super(props);
  }

  public render(): JSX.Element {
    const boxStyle = (() => {
      if(this.props.readonly) {
        return FormEntry.STYLE.box;
      } else {
        return null;
      }
    })();
    const orientation = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return FormEntry.Orientation.VERTICAL;
      } else {
        return FormEntry.Orientation.HORIZONTAL;
      }
    })();
    const paddingSize = (() => {
      switch(this.props.displaySize) {
        case DisplaySize.SMALL:
          return null;
        case DisplaySize.MEDIUM:
          return FormEntry.HORIZONTAL_PADDING;
        case DisplaySize.LARGE:
          return FormEntry.HORIZONTAL_PADDING_BIG;
      }
    })();
    const content = (() => {
      if(orientation === FormEntry.Orientation.HORIZONTAL) {
        return (
          <HBoxLayout style={FormEntry.STYLE.box} width='100%'>
            <div style={FormEntry.STYLE.horizontalHeader}>
              {this.props.name}
            </div>
            <Padding size={paddingSize}/>
            <div style={FormEntry.STYLE.childBox}>{this.props.children}</div>
          </HBoxLayout>);
      } else {
        return (
          <VBoxLayout width='100%'>
            <div style={FormEntry.STYLE.verticalHeader}>{this.props.name}</div>
            <Padding size={FormEntry.VERTICAL_PADDING}/>
            {this.props.children}
          </VBoxLayout>);
      }
    })();
    return (
      <div>
        {content}
      </div>);
  }

  private static STYLE = {
    box: {
      boxSizing: 'border-box' as 'border-box'
    },
    childBox: {
      width: '100%',
      flex: '1 1 auto'
    },
    horizontalHeader: {
      height: '34px',
      width: '130px',
      font: '400 14px Roboto',
      color: '#333333',
      display: 'flex' as 'flex',
      flexDirection: 'row' as 'row',
      flexWrap: 'nowrap' as 'nowrap',
      alignItems: 'center' as 'center',
      flex: '0 0 130px'
    },
    verticalHeader : {
      height: '16px',
      font: '400 14px Roboto',
      color: '#333333',
      paddingLeft: '10px',
      width: '100%'
    }
  };
  private static readonly VERTICAL_PADDING = '12px';
  private static readonly HORIZONTAL_PADDING = '8px';
  private static readonly HORIZONTAL_PADDING_BIG = '40px';
}

export namespace FormEntry {

  /** Lists the ways that a form can be displayed. */
  export enum Orientation {

    /** The form is rendered horizontally. */
    HORIZONTAL,

    /** The form is rendered vertically. */
    VERTICAL
  }
}
