const {logger} = require('../config/logger');

// GET
exports.index = async function (req, res) {
    logger.info('Index');
    return res.render('index.ejs');
};

exports.login = async function (req, res) {
    const identity = req.query.identity;

    // query string 잘못 입력한 경우
    if (identity === 'patient' || identity === 'doctor') {}
    else {
        logger.info(`Error Login - non valid quert string`);
        res.redirect('/');
    }

    logger.info(`Login - ${identity}`);
    return res.render('login.ejs', {id: identity});
};