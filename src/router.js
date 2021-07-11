module.exports = function(app){
    const controller = require('./controller');
    app.get('/', controller.index);
    app.get('/login', controller.login);
};