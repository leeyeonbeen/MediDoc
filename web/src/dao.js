const AWS = require('aws-sdk');
const dynamo_config = require('../config/dynamodb');
AWS.config.update(dynamo_config.aws_remote_config);

exports.dynamoTest = async () => {
    const dynamo = new AWS.DynamoDB.DocumentClient();
    // 파라미터
    const params = {
        TableName: dynamo_config.table_name,
        KeyConditionExpression: 'patientId = :id',
        ExpressionAttributeValues: {
            ':id': 'dsadasdasd'
        }
    }

    const data = await dynamo.query(params).promise();
    return data.Items;
};

// 전체 조회
exports.findAll = async () => {
    const dynamo = new AWS.DynamoDB.DocumentClient();
    // 파라미터
    const params = {
        TableName: dynamo_config.table_name
    };

    const data = await dynamo.scan(params).promise();
    return data.Items;
}