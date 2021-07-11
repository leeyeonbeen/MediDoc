const jwt = require('jsonwebtoken');
const {logger} = require('../config/logger');

require('dotenv').config();
const secret = process.env.JWT_SIGNATURE;

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

exports.patient = async function (req, res) {
    const patientIdx = parseInt(req.params.patientIdx, 10);
    const authUser = parseInt(req.verifiedToken.id, 10);

    // 잘못된 접근 - 환자 인덱스와 토큰의 인덱스가 다를 때
    if (patientIdx !== authUser) {
        logger.info(`Error Patient - patientIdx and token.id are different`);
        return res.redirect('/');
    }

    return res.render('clinic.ejs');
};

exports.doctor = async function (req, res) {
}

exports.loginProcess = async function (req, res) {
    const identity = req.query.identity;
    const code = req.body.inputCode;
    let userIndex = 0;

    // query string 및 inputCode validation check
    if (identity === 'patient') {
        // 지금 환자는 111, 이후에 DB 연결하면 조회하는 걸로 수정
        if (parseInt(code, 10) !== 111) {
            logger.info(`Error Login - non valid patient code`);
            return res.redirect('/login?identity=patient');
        }
        // 임시로 환자 인덱스 설정, DB 연결되면 조회
        userIndex = 117;
    } else if (identity === 'doctor') {
        // 지금 의사는 222, 이후에 DB 연결하면 조회하는 걸로 수정
        if (parseInt(code, 10) !== 222) {
            logger.info(`Error Login - non valid doctor code`);
            return res.redirect('/login?identity=doctor');
        }
        // 임시로 의사 인덱스 설정, DB 연결되면 조회
        userIndex = 319;
    } else {
        logger.info(`Error Login - non valid quert string`);
        return res.redirect('/');
    }

    //토큰 생성
    let token = await jwt.sign({
            id: userIndex,
        },
        secret,
        {
            expiresIn: '2h',
            subject: 'user-info',
        }
    );

    if (identity === 'patient') {
        // 환자 페이지 이동
        return res.redirect(`/patient/${userIndex}?token=${token}`);
    } else {
        // 의사 페이지 이동
        return res.redirect(`/doctor/${userIndex}?token=${token}`);
    }
};