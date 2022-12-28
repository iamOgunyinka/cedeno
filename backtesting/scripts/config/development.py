import os

db_name = 'backtesting_db'
SQLALCHEMY_DATABASE_URI = f'sqlite:///{db_name}.sqlite3'
db_client = os.environ.get('DB_CLIENT', 'sqlite3')

if db_client.find('sqlite3') == -1:
    username = os.environ.get('BT_USERNAME', 'testing')
    password = os.environ.get('BT_PASSWORD', 'testIng007')
    db_name = os.environ.get('DB_NAME', db_name)
    SQLALCHEMY_DATABASE_URI = f'{db_client}://{username}:{password}@localhost/{db_name}'


DEBUG = True
IGNORE_AUTH = True
SQLALCHEMY_TRACK_MODIFICATIONS = False
