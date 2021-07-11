const express = require('./config/express');
const {logger} = require('./config/logger');
require('dotenv').config();

const port = process.env.NODE_PORT || 3000;
express().listen(port);
logger.info(`${process.env.NODE_ENV} - Listening At Port ${port}`);