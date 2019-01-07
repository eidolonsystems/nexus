import * as Beam from 'beam';
import { CurrencyDatabase } from '..';
import { DefinitionsClient } from '.';
import { EntitlementDatabase } from '..';
import { MarketDatabase } from '..';

/** Implements the DefinitionsClient using web services. */
export class WebDefinitionsClient extends DefinitionsClient {
  public get entitlementDatabase(): EntitlementDatabase {
    return this._entitlementDatabase;
  }

  public get currencyDatabase(): CurrencyDatabase {
    return this._currencyDatabase;
  }

  public get marketDatabase(): MarketDatabase {
    return this._marketDatabase;
  }

  public async open(): Promise<void> {
    const entitlementResponse = await Beam.post(
      '/api/administration_service/load_entitlements_database', {});
    this._entitlementDatabase = EntitlementDatabase.fromJson(
      entitlementResponse);
    const currencyResponse = await Beam.post(
      '/api/definitions_service/load_currency_database', {});
    this._currencyDatabase = CurrencyDatabase.fromJson(currencyResponse);
    const marketResponse = await Beam.post(
      '/api/definitions_service/load_market_database', {});
    this._marketDatabase = MarketDatabase.fromJson(marketResponse);
  }

  public async close(): Promise<void> {
    return;
  }

  private _entitlementDatabase: EntitlementDatabase;
  private _currencyDatabase: CurrencyDatabase;
  private _marketDatabase: MarketDatabase;
}
