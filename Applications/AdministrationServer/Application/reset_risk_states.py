import argparse
import sys

import yaml

import beam
import nexus

def report_yaml_error(error):
  if hasattr(error, 'problem_mark'):
    sys.stderr.write('Invalid YAML at line %s, column %s: %s\n' % \
      (error.problem_mark.line, error.problem_mark.column, str(error.problem)))
  else:
    sys.stderr.write('Invalid YAML provided\n')

def parse_ip_address(source):
  separator = source.find(':')
  if separator == -1:
    return beam.network.IpAddress(source, 0)
  return beam.network.IpAddress(source[0:separator],
    int(source[separator + 1 :]))

def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2019 Eidolon Systems Ltd.')
  parser.add_argument('-c', '--config', type=str, help='Configuration file',
    default='config.yml')
  args = parser.parse_args()
  try:
    stream = open(args.config, 'r').read()
    config = yaml.load(stream, yaml.SafeLoader)
  except IOError:
    sys.stderr.write('%s not found\n' % args.config)
    exit(1)
  except yaml.YAMLError as e:
    report_yaml_error(e)
    exit(1)
  section = config['service_locator']
  address = parse_ip_address(section['address'])
  username = section['username']
  password = section['password']
  service_locator_client = beam.service_locator.ApplicationServiceLocatorClient(
    address)
  service_locator_client.set_credentials(username, password)
  service_locator_client.open()
  administration_client = \
    nexus.administration_service.ApplicationAdministrationClient(
    service_locator_client)
  administration_client.open()
  for account in service_locator_client.load_all_accounts():
    administration_client.store_risk_state(account,
      nexus.risk_service.RiskState())

if __name__ == '__main__':
  main()
