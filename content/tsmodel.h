#ifndef TSMODEL_H
#define TSMODEL_H

#include <QAbstractListModel>
#include <qqml.h>
#include "task.h"
#include <QFile>

#include <random>

//The tutorial only showed Qt::UserRole, but anything incremented beyond it will work. It appears to be a constant, so we can use it in an expression defining the others.
namespace {
enum TaskRoles{
    TaskJobRole = Qt::UserRole, // All values below Qt::UserRole are reserved.
    TaskPhaseRole = Qt::UserRole+1,
    TaskNameRole = Qt::UserRole+2,
    TaskCompleteRole = Qt::UserRole+3
};
}

//The model interfaces to exposed_task as if it were a single element.
//The vector of tasks is private, and exposed_task is populated from it when events trigger.
class TSModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit TSModel(QAbstractListModel *parent = nullptr);
    //void ReadDatabase(QString filePath);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int columnCount(const QModelIndex &parent) const override;
    Q_INVOKABLE void doSomething();
    Q_INVOKABLE bool getIsJobComplete(int n);
    Q_INVOKABLE bool changeJob();
    Q_INVOKABLE QString getCurrentJob();
    Q_INVOKABLE QString getCurrentTask();


public slots:
    Q_INVOKABLE void completeTask();
    Q_INVOKABLE void changeTask();

signals:


private:
    //int phaseIndices[10];
    int currentPhase = 1;
    int highestPhase = 1;
    int currentTaskIndex = 0;
    std::vector<Task> task_list;
    Task exposed_task;
    void SortTaskVector(); //In Place sorts the task vector by phase, the by task. Bubble sort because it's easy.
    void PrintTaskVector();
    void PrintTask(Task t);
    bool isJobComplete = false;
    //bool isPreviousJobComplete = true;


    // QAbstractItemModel interface

    // QAbstractItemModel interface
public:
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};
#endif // TSMODEL_H
