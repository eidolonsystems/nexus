import * as React from 'react';
import * as Nexus from 'nexus';
import * as Beam from 'beam';
import { DisplaySize } from '../../../display_size';
import {CurrencySelectionBox, MoneyInputBox, TextInputField, IntegerInputBox} from '../../../components';
import { ComplianceParameter, ComplianceValue } from 'nexus';

interface Properties {
  displaySize: DisplaySize;
  parameter?: Nexus.ComplianceParameter;
  currencyDatabase: Nexus.CurrencyDatabase;
  onChange?: (newParameter: Nexus.ComplianceParameter) => void;
}



export class ParameterEntry extends React.Component<Properties> {
  constructor(props: Properties) {
    super(props);
    this.onChange = this.onChange.bind(this);
  }

  public render(): JSX.Element {
     const rowStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParameterEntry.STYLE.rowSmall;
      } else {
        return ParameterEntry.STYLE.rowLarge;
      }
    })();
    const labelStyle = (() => {
      if(this.props.displaySize === DisplaySize.SMALL) {
        return ParameterEntry.STYLE.label;
      } else {
        return ParameterEntry.STYLE.label;
      }
    })();
    const input = (() => {
      switch(this.props.parameter.value.type) {
        case Nexus.ComplianceValue.Type.BOOLEAN:
          return <div/>;
        case Nexus.ComplianceValue.Type.CURRENCY:
          return (
            <CurrencySelectionBox
              value={this.props.parameter.value.value}
              onChange={(newValue: Nexus.Currency) => this.onChange(newValue)}
              currencyDatabase={this.props.currencyDatabase}/>);
        case Nexus.ComplianceValue.Type.DATE_TIME:
          return <div/>;
        case Nexus.ComplianceValue.Type.DOUBLE:
          return <div/>;
        case Nexus.ComplianceValue.Type.DURATION:
          return <div/>;
        case Nexus.ComplianceValue.Type.MONEY:
          return (
            <MoneyInputBox
              onChange={(newValue: Nexus.Money) => this.onChange(newValue)} 
              value={this.props.parameter.value.value}/>);
        case Nexus.ComplianceValue.Type.QUANTITY:
          return (
            <IntegerInputBox
              onChange={(newValue: number) => this.onChange(newValue)}
              value={this.props.parameter.value.value}/>);
        case Nexus.ComplianceValue.Type.SECURITY:
          return <div/>;
        case Nexus.ComplianceValue.Type.STRING:
          return (
            <TextInputField
              displaySize ={this.props.displaySize}
              value={this.props.parameter.value.value}
              onInput={(newValue: string) => this.onChange(newValue)}/>);
        case Nexus.ComplianceValue.Type.LIST:
          const list = [] as any[];
          for(const thing of this.props.parameter.value.value) {
            list.push(
              <ParameterEntry
                currencyDatabase={this.props.currencyDatabase}
                displaySize={this.props.displaySize}
                parameter={thing}/>);
          }
        case Nexus.ComplianceValue.Type.NONE:
          return <div/>
      }
    })();
    return (
      <div style={rowStyle}>
        <div style={labelStyle}>{this.props.parameter.name}</div>
        <div style={{flexGrow: 1}}>
        {input}
        </div>
      </div>);
  }

  private onChange(newValue: any) {
    this.props.onChange(
      new ComplianceParameter(
        this.props.parameter.name, 
        new ComplianceValue(this.props.parameter.value.type, newValue)));
  }

    private static readonly STYLE = {
      largeWrapper: {
        paddingLeft: '38px'
      },
      rowSmall: {
        display: 'flex' as 'flex',
        flexDirection: 'row' as 'row',
        minWidth: '284px',
        maxWidth: '424px',
        width: '100%',
        height: '34px',
        font: '400 16px Roboto',
        alignItems: 'center' as 'center'
      },
      rowLarge: {
        display: 'flex' as 'flex',
        flexDirection: 'row' as 'row',
        width: '100%',
        height: '34px',
        font: '400 14px Roboto',
        alignItems: 'center' as 'center'
      },
      fillerBetweenRows : {
        height: '10px',
        width: '100%'
      },
      bottomFiller: {
        height: '30px'
      },
      label: {
        width: '100px',
        font: '400 14px Roboto',
      },
      headerSmall: {
        color: '#4B23A0',
        font: '500 14px Roboto',
        marginTop: '10px',
        marginBottom: '18px'
      },
      headerLarge: {
        color: '#4B23A0',
        font: '500 14px Roboto',
        marginTop: '10px',
        marginBottom: '18px'
      }
    };
}

