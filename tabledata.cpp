#include "tabledata.h"

TableData::TableData() : status(0) {}

TableData::~TableData() {}

void TableData::setStatus(int status) {
    this->status = status;
}

void TableData::setMsg(const QString& msg) {
    this->msg = msg;
}

void TableData::addColumn(const QString& column, const QString& type) {
    columns[column] = type;
}

void TableData::addRow(const QMap<QString, QString>& rowData) {
    rows.append(rowData);
}

QJsonObject TableData::toJsonObject() const {
    QJsonObject root;
    root["status"] = status;
    root["msg"] = msg;
    
    // 添加列信息
    QJsonObject columnsObj;
    for (auto it = columns.constBegin(); it != columns.constEnd(); ++it) {
        columnsObj[it.key()] = it.value();
    }
    root["columns"] = columnsObj;
    
    // 添加行数据
    QJsonArray rowsArray;
    for (const auto& row : rows) {
        QJsonObject rowObj;
        for (auto it = columns.constBegin(); it != columns.constEnd(); ++it) {
            rowObj[it.key()] = row.value(it.key(), "");
        }
        rowsArray.append(rowObj);
    }
    root["rows"] = rowsArray;
    
    return root;
}

QString TableData::toJson() const {
    QJsonDocument doc(toJsonObject());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
} 