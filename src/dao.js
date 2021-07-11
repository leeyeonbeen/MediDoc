const AWS = require('aws-sdk');
const dynamo_config = require('../config/dynamodb');
AWS.config.update(dynamo_config.aws_remote_config);

exports.dynamoTest = async () => {
    const dynamo = new AWS.DynamoDB.DocumentClient();
    const params = {
        TableName: dynamo_config.table_name,
        KeyConditionExpression: 'patientId = :id',
        ExpressionAttributeValues: {
            ':id': 'dsadasdasd'
        }
    }

    await dynamo.query(params, (err, data) => {
        if (err) {
            console.log(err);
        } else {
            const { Items } = data;
            console.log(Items);
        }
    });
};