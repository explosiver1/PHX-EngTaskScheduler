#include "tsmodel.h"

TSModel::TSModel(QAbstractListModel *parent): QAbstractListModel{parent}
{
    Task t;
    QFile file = QFile("C:/Users/jholston/Documents/Task.csv");
    if (!file.exists()){
        qDebug() << "File cannot be found ";
    }else{
        qDebug() << " Opening...";
    }
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&file);
        int i = 0;
        while (!stream.atEnd()) {
            line = stream.readLine();
            QStringList lineByParts = line.split(',');
            if (lineByParts.length() == 4) {
                task_list.push_back(Task());
                task_list.at(i).job = lineByParts[0];
                task_list.at(i).phase = lineByParts[1].toInt();
                task_list.at(i).name = lineByParts[2];
                qDebug() << task_list.at(i).name;
                int check = lineByParts[3].toInt();
                if (check == 1) {
                    task_list.at(i).isComplete = true;
                } else {
                    task_list.at(i).isComplete = false;
                }
                i++;
            }
        }
        file.close();
        QString msg = "Task List Size: " + QString::number(task_list.size());
        qDebug() << msg;
        SortTaskVector();
        highestPhase = task_list[task_list.size() - 1].phase; //bc it's sorted, this will always be the highest phase.
        qDebug() << "Highest Phase: " + QString::number(highestPhase);
        PrintTaskVector();
        //ChangeTask(); //This calls a virtual method internally, so it shouldn't run in the constructor.
    } else {
        qDebug() << "File could not be opened.";
    }
}

int TSModel::rowCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant TSModel::data(const QModelIndex &index, int role) const
{
    //I've commented this out because we really don't care about the index. We only care about role because it defines what data should be returned from exposed_task.
    /*
    if (!index.isValid()) {
        return {}; // If an index is invalid, we return invalid data.
    }*/
    Task task = exposed_task;
    if (role == TaskJobRole) {
        return task.job;
    } else if (role == TaskPhaseRole) {
        return task.phase;
    } else if (role == TaskNameRole) {
        return task.name;
    } else if (role == TaskCompleteRole){
        return task.isComplete;
    } else  {
        return QVariant();
    }
}

//These role names provide access in QML.
QHash<int, QByteArray> TSModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[TaskJobRole] =  "job";
    roles[TaskPhaseRole] = "phase";
    roles[TaskNameRole] = "name";
    roles[TaskCompleteRole] = "isComplete";
    return roles;
}

QModelIndex TSModel::index(int row, int column, const QModelIndex &parent) const
{
    //Using createIndex() instead of the QModelIndex() constructor allowed the UI to work right.
    //QModelIndex() returns an invalid index, which the QML bindings don't like. The code that subscribes the view to the property it probably filtering invalid Indexes.
    return QAbstractListModel::createIndex(0,0,nullptr);
}

QModelIndex TSModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

int TSModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

//Test function for calling from QML.
void TSModel::doSomething()
{
    qDebug() << "Doing something in C++";
}

bool TSModel::getIsJobComplete(int n)
{
    //I thought I needed lookback, but I realized I can just change when the functions are called.
    return isJobComplete;
    /*
    if (n == 1){
        return isJobComplete;
    } else if (n == 0) {
        return isPreviousJobComplete;
    } else {
        return false;
    } */

}

bool TSModel::changeJob()
{
    //Placeholder
    return true;
}

QString TSModel::getCurrentJob()
{
    return exposed_task.job;
}

QString TSModel::getCurrentTask()
{
    return exposed_task.name;
}


void TSModel::completeTask()
{
    setData(QModelIndex(), true, TaskCompleteRole);
    //Assigning it here too because the actual data set and model interface are decoupled.
    task_list[currentTaskIndex].isComplete = true;
    bool incPhase = true;
    for (int i = 0; i < task_list.size(); i++) {
        if (task_list[i].phase == currentPhase && task_list[i].isComplete == false) {
            incPhase = false;
            qDebug() << "incPhase false.";
        }
    }
    if (incPhase && currentPhase < highestPhase) {
        currentPhase++;
        qDebug() << "Phase incremented.";
    } else if (incPhase && currentPhase >= highestPhase) {
        isJobComplete = true;
        qDebug() << "This job is done";
    }


    QString msg = "Task " + task_list[currentTaskIndex].name + " Completed.";
    if (incPhase) {
        msg = msg + " Entering phase " + QString::number(currentPhase);
    }
    qDebug() << msg;
}

void TSModel::changeTask()
{
    qDebug() << "Phase: " +  QString::number(currentPhase) + " Changing task...";
    bool phaseComplete = true;
    int phaseStartIndex = -1, phaseEndIndex = -1;
    for (int i = 0; i < task_list.size(); i++) {
        //Start by checking if the current phase is complete and something is messed up.
        if (task_list[i].phase == currentPhase && task_list[i].isComplete == false && phaseComplete) {
            phaseComplete = false;
            qDebug() << "Change Task: Phase Incomplete";
            qDebug() << "Phase: " + QString::number(task_list[i].phase) + ", Task: " + task_list[i].name + ", Completion: " + QString::number(task_list[i].isComplete);
        }

        if(phaseStartIndex == -1 && task_list[i].phase == currentPhase) {
            phaseStartIndex = i; //Only gets set once because it should be the first match only.

        }
        //This needs to not be connected because the start and end index may be the same position.
        if (i < task_list.size() - 1 && task_list[i].phase == currentPhase && task_list[i+1].phase != currentPhase) {
            phaseEndIndex = i; //Can be set from either a transition to another phase (this check) or the end of the array (next check).
        } else if ( i == task_list.size() - 1 && task_list[i].phase == currentPhase) {
            phaseEndIndex = i;
        }
    }

    qDebug() << "PhaseStartIndex: " + QString::number(phaseStartIndex);
    qDebug() << "PhaseEndIndex: " + QString::number(phaseEndIndex);

    //Random number generation code from <random>.
    //It acts through a lot of side effects and internal variables, so it's hard to parse.
    if (phaseStartIndex >= 0 && phaseEndIndex >= 0 && phaseEndIndex >= phaseStartIndex && !phaseComplete) {
        while(task_list[currentTaskIndex].isComplete && !phaseComplete){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> distrib(phaseStartIndex, phaseEndIndex);
            currentTaskIndex = distrib(gen);
        }
        //setData is the method QML is looking for when reacting to dataChanged(). Not sure if it's necessary, but I'll stick with the convention. dataChanged() is 100% needed.
        setData(index(0,0), task_list[currentTaskIndex].job, TaskJobRole);
        setData(index(0,0), task_list[currentTaskIndex].phase, TaskPhaseRole);
        setData(index(0,0), task_list[currentTaskIndex].name, TaskNameRole);
        setData(index(0,0), task_list[currentTaskIndex].isComplete, TaskCompleteRole);
    } else if (phaseComplete) {
        qDebug() << "completeTask missed this one.";
    }    else {
        qDebug() << "Error, could not find viable phase range.";
    }

    PrintTask(exposed_task);
}

//Not an efficient sort, but only needs to be done at initialization on a small array.
void TSModel::SortTaskVector() {
    qDebug() << "Sorting Task Vector...";
    Task tmp;
    for (int i = 0; i < task_list.size() - 1; i++) {
        for (int j = i + 1; j < task_list.size(); j++) {
            if (task_list[i].phase > task_list[j].phase) {
                tmp = task_list[j];
                task_list[j] = task_list[i];
                task_list[i] = tmp;
            }
        }
    }

}

void TSModel::PrintTaskVector() {
    for (int i = 0; i < task_list.size(); i++) {
        PrintTask(task_list[i]);
    }
}

void TSModel::PrintTask(Task t) {
    if (t.job != nullptr) {
        QString toPrint = "Job: " + t.job + ", Phase: " + QString::number(t.phase) + ", Name: " + t.name + ", isComplete: " + QString::number(t.isComplete);
        qDebug() << toPrint;
    }
}

bool TSModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    // Depending on the role, update the corresponding data field
    switch (role) {
    case TaskJobRole:
        if (exposed_task.job != value.toString()) {
            exposed_task.job = value.toString();
            emit dataChanged(index, index, {role}); // Emit dataChanged signal for the specific role
            return true;
        }
        break;
    case TaskNameRole:
        if (exposed_task.name != value.toString()) {
            exposed_task.name = value.toString();
            emit dataChanged(index, index, {role}); // Emit dataChanged signal for the specific role
            return true;
        }
        break;
    case TaskPhaseRole:
        if (exposed_task.phase != value.toInt()) {
            exposed_task.phase = value.toInt();
            emit dataChanged(index, index, {role}); // Emit dataChanged signal for the specific role
            return true;
        }
        break;
        // Handle more roles as needed
    case TaskCompleteRole:
        if (exposed_task.isComplete != value.toBool()) {
            exposed_task.isComplete = value.toBool();
            emit dataChanged(index, index, {role});
            return true;
        }
    }

    return false;
}






