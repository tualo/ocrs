#!/bin/sh


find_vars () {
  read -p "Please enter the remote database host:" hostname
  read -p "Please enter the db name for the remote database:" dbname
  read -p "Please enter the username for the remote database:" username
  read -p "Please enter the password for the remote database:" password

  echo "Your remote system settings: "
  echo "  hostname: $hostname"
  echo "  db: $dbname"
  echo "  username: $username"
  echo "  password: ****"

  read -p "Please enter the db name for the local database:" localdbname
  read -p "Please enter the username for the local database:" localusername
  read -p "Please enter the password for the local database:" localpassword

  echo "install plugin federated soname 'ha_federated.so';" | mysql --force=true -u $localusername -p$localpassword $localdbname

  sed -e "s/HOST/$hostname/g" ddl.sql | \
  sed -e "s/DBNAME/$dbname/g" | \
  sed -e "s/USER/$username/g" | \
  sed -e "s/PORT/3306/g" | \
  sed -e "s/PASSWORD/$password/g" | \
  mysql -u $localusername -p$localpassword $localdbname

};

run () {
  read -p "This program configures the scanner database, do you whish to continue?" yn
  case $yn in
      [Yy]* ) find_vars; break;;
      [Nn]* ) exit;;
      * ) echo "Please answer yes or no.";;
  esac
};

run
