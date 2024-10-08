#!/usr/bin/env python3
import os
from flask import Flask
from flask_migrate import Migrate
from flask_sqlalchemy import SQLAlchemy

migrate = Migrate()
db = SQLAlchemy()


class User(db.Model):
	__tablename__ = 'bt_users'
	id = db.Column(db.Integer, primary_key=True, unique=True, index=True)
	username = db.Column(db.String(16), unique=True, index=True)
	assets = db.relationship('UserAsset', backref='user', lazy='dynamic', cascade='all,delete')
	# trades = db.relationship('Trade', backref='user', lazy='dynamic', cascade='all,delete')


class UserAsset(db.Model):
	__tablename__ = 'bt_user_assets'
	id= db.Column(db.Integer, primary_key=True, unique=True, index=True)
	ownerID = db.Column(db.Integer, db.ForeignKey('bt_users.id'), nullable=False)
	tokenName = db.Column(db.String(16), nullable=False, unique=True)
	amountInUse = db.Column(db.Float, default=0.0)
	amountAvailable = db.Column(db.Float, default=0.0)


class Symbol(db.Model):
	__tablename__ = 'bt_symbols'
	id= db.Column(db.Integer, primary_key=True, unique=True, index=True)
	tokenName = db.Column(db.String(32), nullable=False, unique=False)
	baseToken = db.Column(db.String(16), nullable=True, unique=False)
	quoteToken = db.Column(db.String(16), nullable=True, unique=False)
	tradeType = db.Column(db.Integer)

class Order(db.Model):
	__tablename__ = 'bt_orders'
	id = db.Column(db.Integer, primary_key=True, unique=True, index=True)
	tokenName = db.Column(db.String(16), nullable=False, unique=False)
	orderID = db.Column(db.BigInteger, nullable=False)
	userID = db.Column(db.Integer, db.ForeignKey('bt_users.id'), nullable=False)
	side = db.Column(db.SmallInteger, nullable=False, unique=False)
	orderType = db.Column(db.SmallInteger, nullable=False, unique=False)
	market = db.Column(db.SmallInteger, nullable=False, unique=False)
	status = db.Column(db.SmallInteger, nullable=False, unique=False)
	quantity = db.Column(db.Float, nullable=False, unique=False)
	priceLevel = db.Column(db.Float, nullable=False, default=0.0, unique=False)
	leverage = db.Column(db.SmallInteger, nullable=False, default=1, unique=False)


class Trade(db.Model):
	__tablename__ = 'bt_trades'
	id = db.Column(db.Integer, primary_key=True, unique=True, index=True)
	symbol = db.Column(db.String(16), nullable=False)
	tradeID = db.Column(db.BigInteger, nullable=False)
	orderID = db.Column(db.Integer, nullable=False)
	userID = db.Column(db.Integer, db.ForeignKey('bt_users.id'), nullable=False)
	side = db.Column(db.SmallInteger, nullable=False, unique=False)
	tradeType = db.Column(db.SmallInteger, nullable=False, unique=False)
	status = db.Column(db.SmallInteger, nullable=False, unique=False)
	eventTime = db.Column(db.Integer, nullable=False, default=0, unique=False)
	quantityExec = db.Column(db.Float, nullable=False, default=0.0, unique=False)
	amountPerPiece = db.Column(db.Float, nullable=False, default=0.0, unique=False)



def create_app(config_name):
	app = Flask(__name__)
	# apply configuration
	cfg = os.path.join(os.getcwd(), 'config', config_name + '.py')
	app.config.from_pyfile(cfg)
	
	# initialize extensions
	db.init_app(app)
	migrate.init_app(app, db)
	return app


app = create_app('development')

if __name__ == '__main__':
	with app.app_context():
		db.drop_all()
		db.create_all()
		print('Created')
