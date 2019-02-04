import * as Beam from 'beam';
import { AccountDirectoryModel } from './account_directory_model';
import { AccountEntry } from './account_entry';

/** Implements an AccountDirectoryModel in memory. */
export class LocalAccountDirectoryModel extends AccountDirectoryModel {

  /** Constructs an new model.
   * @param groups - A set of groups.
   * @param accounts - A map of all the accounts associated with
   *        each group.
   */
  constructor(groups: Beam.Set<Beam.DirectoryEntry>,
      accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>) {
    super();
    this._isLoaded = false;
    this._groups = groups.clone();
    this._accounts = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
    for(const group of this._groups) {
      this._accounts.set(group, accounts.get(group).slice());
    }
  }

  /** Returns true if this model has been loaded. */
  public isLoaded(): boolean {
    return this._isLoaded;
  }

  public async load(): Promise<void> {
    this._isLoaded = true;
  }

  public get groups(): Beam.Set<Beam.DirectoryEntry> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return this._groups.clone();
  }

  public async loadAccounts(
      group: Beam.DirectoryEntry): Promise<AccountEntry[]> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return new Promise<AccountEntry[]>((resolve) => {
      setTimeout(() => {
        resolve(this._accounts.get(group).slice());}, 100);
      });
  }
  public async loadFilteredAccounts(
      filter: string): Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>> {
    if(!this.isLoaded) {
      throw Error('Model not loaded.');
    }
    return new Promise<Beam.Map<Beam.DirectoryEntry, AccountEntry[]>>(
      (resolve) => {
        setTimeout(() => {
          const map = new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>();
          if(filter) {
            for(const group of this._groups) {
              const accounts: AccountEntry[] = [];
              for(const account of this._accounts.get(group)) {
                if(account.account.name.indexOf(filter) === 0) {
                  accounts.push(account);
                }
              }
              map.set(group, accounts);
            }
          }
          resolve(map);}, 100);
      });
  }

  private _isLoaded: boolean;
  private _groups: Beam.Set<Beam.DirectoryEntry>;
  private _accounts: Beam.Map<Beam.DirectoryEntry, AccountEntry[]>;
}
