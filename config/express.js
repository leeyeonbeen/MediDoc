const express = require('express');
const cookieParser = require('cookie-parser');
const compression = require('compression');
const methodOverride = require('method-override');
var cors = require('cors');
module.exports = function () {
    const app = express();
    app.set("view engine", "ejs");
    app.set("views", process.cwd() + "/views");

    app.use(cookieParser());
    app.use(compression());
    app.use(express.json());
    app.use(express.urlencoded({extended: true}));
    app.use(methodOverride());
    app.use(cors());
    app.use(express.static(process.cwd() + '/public'));

    app.set("view engine", "ejs");
    app.set("views", process.cwd() + "/views");
    app.use(express.static(process.cwd() + '/static'));

    // Routing configure
    require('../src/router')(app);
    
    return app;
};