import * as Beam from 'beam';
import * as Nexus from 'nexus';
import { AccountDirectoryModel, AccountEntry, HttpAccountDirectoryModel,
  HttpAccountModel, LocalAccountDirectoryModel } from '..';
import { DashboardModel } from './dashboard_model';
import { LocalDashboardModel } from './local_dashboard_model';

/** Implements the DashboardModel using HTTP requests. */
export class HttpDashboardModel extends DashboardModel {

  /** Constructs an HttpDashboardModel.
   * @param serviceClients - The service clients used to access the HTTP
   *        services.
   */
  constructor(serviceClients: Nexus.ServiceClients) {
    super();
    this.serviceClients = serviceClients;
    this.model = new LocalDashboardModel(Beam.DirectoryEntry.INVALID,
      new Nexus.AccountRoles(0), new Nexus.EntitlementDatabase(),
      new Nexus.CountryDatabase(), new Nexus.CurrencyDatabase(),
      new Nexus.MarketDatabase(), new LocalAccountDirectoryModel(
      new Beam.Set<Beam.DirectoryEntry>(),
      new Beam.Map<Beam.DirectoryEntry, AccountEntry[]>()));
  }

  public get entitlementDatabase(): Nexus.EntitlementDatabase {
    return this.model.entitlementDatabase;
  }

  public get countryDatabase(): Nexus.CountryDatabase {
    return this.model.countryDatabase;
  }

  public get currencyDatabase(): Nexus.CurrencyDatabase {
    return this.model.currencyDatabase;
  }

  public get marketDatabase(): Nexus.MarketDatabase {
    return this.model.marketDatabase;
  }

  public get account(): Beam.DirectoryEntry {
    return this.model.account;
  }

  public get roles(): Nexus.AccountRoles {
    return this.model.roles;
  }

  public get accountDirectoryModel(): AccountDirectoryModel {
    return this.model.accountDirectoryModel;
  }

  public makeAccountModel(account: Beam.DirectoryEntry): HttpAccountModel {
    return new HttpAccountModel(account, this.serviceClients);
  }

  public async load(): Promise<void> {
    if(this.model.isLoaded) {
      return;
    }
    await this.serviceClients.open();
    const account = await
      this.serviceClients.serviceLocatorClient.loadCurrentAccount();
    const roles = await
      this.serviceClients.administrationClient.loadAccountRoles(account);
    this.model = new LocalDashboardModel(account, roles,
      this.serviceClients.definitionsClient.entitlementDatabase,
      this.serviceClients.definitionsClient.countryDatabase,
      this.serviceClients.definitionsClient.currencyDatabase,
      this.serviceClients.definitionsClient.marketDatabase,
      new HttpAccountDirectoryModel(account, this.serviceClients));
    return this.model.load();
  }

  public async logout(): Promise<void> {
    this.serviceClients.close();
  }

  private serviceClients: Nexus.ServiceClients;
  private model: LocalDashboardModel;
}
