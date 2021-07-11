module.exports = function(app){
    const controller = require('./controller');
    const auth = require('../config/auth');

    app.get('/', controller.index);
    app.get('/login', controller.login);
    app.get('/patient/:patientIdx', auth, controller.patient);
    app.get('/doctor/:doctorIdx', auth, controller.doctor);
    app.post('/loginProcess', controller.loginProcess);
};