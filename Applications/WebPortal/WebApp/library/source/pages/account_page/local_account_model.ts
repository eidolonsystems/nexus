import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountModel } from './account_model';

/** Implements an AccountModel locally. */
export class LocalAccountModel extends AccountModel {

  /** Constructs a LocalAccountModel. */
  constructor(account: Beam.DirectoryEntry, roles: Nexus.AccountRoles) {
    super();
    this._isLoaded = false;
    this._account = account;
    this._roles = roles;
  }

  /** Returns true of this model has been loaded. */
  public get isLoaded(): boolean {
    return this._isLoaded;
  }

  public get account(): Beam.DirectoryEntry {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._account;
  }

  public get roles(): Nexus.AccountRoles {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._roles;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  private _isLoaded: boolean;
  private _account: Beam.DirectoryEntry;
  private _roles: Nexus.AccountRoles;
}
