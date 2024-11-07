#ifndef TABLEDATA_H
#define TABLEDATA_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

/**
 * @brief 数据表结构类，用于存储数据库查询结果
 * 包含查询状态、消息、列信息和行数据
 */
class TableData {
public:
    TableData();
    ~TableData();
    
    /**
     * @brief 设置查询状态
     * @param status 状态码（0表示成功，非0表示失败）
     */
    void setStatus(int status);

    /**
     * @brief 设置查询结果消息
     * @param msg 结果消息
     */
    void setMsg(const QString& msg);

    /**
     * @brief 添加列名和对应的数据类型
     * @param column 列名
     * @param type 数据类型
     */
    void addColumn(const QString& column, const QString& type);

    /**
     * @brief 添加一行数据
     * @param rowData 一行的数据，key为列名，value为值
     */
    void addRow(const QMap<QString, QString>& rowData);

    /**
     * @brief 将查询结果序列化为JSON字符串
     * @return JSON格式的字符串
     */
    QString toJson() const;

    /**
     * @brief 将查询结果转换为QJsonObject
     * @return QJsonObject对象
     */
    QJsonObject toJsonObject() const;

    /**
     * @brief 获取查询状态
     * @return 查询状态码
     */
    int getStatus() const { return status; }

    /**
     * @brief 获取查询消息
     * @return 查询消息
     */
    QString getMsg() const { return msg; }

    /**
     * @brief 获取列信息
     * @return 列名和类型的映射
     */
    const QMap<QString, QString>& getColumns() const { return columns; }

    /**
     * @brief 获取行数据
     * @return 行数据数组
     */
    const QVector<QMap<QString, QString>>& getRows() const { return rows; }

private:
    int status;                     // 查询状态
    QString msg;                    // 查询消息
    QMap<QString, QString> columns; // 列名和类型的映射
    QVector<QMap<QString, QString>> rows;  // 行数据数组
}; 

#endif // TABLEDATA_H 