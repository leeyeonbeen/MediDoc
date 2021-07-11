const jwt = require('jsonwebtoken');
require('dotenv').config();
const secret = process.env.JWT_SIGNATURE;
const auth = (req, res, next) => {
    const token = req.query.token;
    
    // 토큰이 없을 때
    if(!token) { res.redirect('/'); }

    // 토큰 확인
    const p = new Promise(
        (resolve, reject) => {
            jwt.verify(token, secret , (err, verifiedToken) => {
                if(err) reject(err);
                resolve(verifiedToken)
            })
        }
    );

    // 유효하지 않은 토큰일 때
    const onError = (error) => { res.redirect('/'); };

    // process the promise
    p.then((verifiedToken)=>{
        req.verifiedToken = verifiedToken;
        next();
    }).catch(onError)
};

module.exports = auth;