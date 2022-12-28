#!/usr/bin/env

python -m pip install --upgrade pip
pip install flask
pip install flask_sqlalchemy
pip install flask_migrate
pip install mysqlclient

# make sure you create an odbc with the name 'backtesting' and the following parameters
# create user 'testing'@'localhost' identified by 'testIng007';
# create database backtesting_db;
# grant all privileges on backtesting_db.* to 'testing'@'localhost';
# FLUSH PRIVILEGES;