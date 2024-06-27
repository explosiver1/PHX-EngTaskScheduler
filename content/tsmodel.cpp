#include "tsmodel.h"

TSModel::TSModel(QAbstractListModel *parent): QAbstractListModel{parent}
{
    //This snippet opens the Docouments folder on Windows.
    //QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QFile f = QFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "ETS.txt");
    if (f.exists()) {
        if(f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString jobNum = f.readLine();
            //File in user documents folder holds the initial job number to get back to.
            initJobPath = jobFolderPath + jobNum + ".csv";
        } else {
            qDebug() << "File could not be opened";
        }
    } else {
        qDebug() << "File could not be found";
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

//Split this from the constructor because it needs to be called beyond initialization.
//Initialization is running this with the job # in the user's save.
//s is now the job name and not the full path. This gives me a little more flexibility to not put the job name as a parameter in the template.
bool TSModel::changeJob(QString s)
{
    QString path = jobFolderPath + s + ".csv";
    qDebug() << "Entering changeJob() with parameter " + s;
    task_list.clear();
    currentPhase = 1;
    highestPhase = 1;
    Task t;
    QFile file = QFile(path);
    if (!file.exists()){
        qDebug() << "File cannot be found ";
        return false;
    }else{
       // qDebug() << " Opening...";
        QString line;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            int i = 0;
            isJobComplete = true;
            //Read every task and check for completion.
            while (!stream.atEnd()) {
                line = stream.readLine();
                QStringList lineByParts = line.split(',');
                if (lineByParts.length() == 3) {
                    task_list.push_back(Task());
                    task_list.at(i).job = s;
                    task_list.at(i).phase = lineByParts[0].toInt();
                    task_list.at(i).name = lineByParts[1];
                    //qDebug() << task_list.at(i).name;
                    int check = lineByParts[2].toInt();
                    if (check == 1) {
                        task_list.at(i).isComplete = true;
                    } else {
                        task_list.at(i).isComplete = false;
                        isJobComplete = false;
                    }
                    i++;
                }
            }
            file.close();
            QString msg = "Task List Size: " + QString::number(task_list.size());
           // qDebug() << msg;
            SortTaskVector();
            highestPhase = task_list[task_list.size() - 1].phase; //bc it's sorted, this will always be the highest phase.
            //qDebug() << "Highest Phase: " + QString::number(highestPhase);
            currentPhase = highestPhase;
            //Find Current Phase
            for(int i = 0; i < task_list.size() && currentPhase == highestPhase; i++) {
                if (task_list[i].isComplete != true) {
                    currentPhase = task_list[i].phase;
                }
            }
            PrintTaskVector();
            return true;
            //ChangeTask(); //This calls a virtual method internally, so it shouldn't run in the constructor.
        } else {
            qDebug() << "File could not be opened.";
            return false;
        }
    }

    //Add a block here to save the current job number to the ETS.csv file in the user's documents folder.
}

QString TSModel::getCurrentJob()
{
    return exposed_task.job;
}

QString TSModel::getCurrentTask()
{
    return exposed_task.name;
}

bool TSModel::createJob(QString s)
{
    QString path = jobFolderPath + s + ".csv";
    qDebug() << "Entering createJob()";
    if (QFile::copy(jobTemplatePath, path)) {
        return true;
    } else {
        qDebug() << "Job creation failed. Template could not be copied to job name";
        return false;
    }

    /*
    QFile file(s);
    if (file.open(QIODevice::ReadWrite)) {
        file.resize(0);
        QTextStream out(&file);
        out << "wee";
        file.close();
        return true;
    } else {
        return false;
    }*/
}

QString TSModel::getInitJobPath()
{
    return initJobPath;
}

QString TSModel::getJobFolderPath()
{
    return jobFolderPath;
}

bool TSModel::saveJob()
{
    qDebug() << "Entering saveJob()";
    QString path = jobFolderPath + exposed_task.job + ".csv";
    QFile file = QFile(path);
    if (!file.exists()){
        qDebug() << "File cannot be found  at: " + path;
        return false;
    }else{
        // qDebug() << " Opening...";
        QString line;
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)){
            QTextStream out(&file);
            for (int i = 0; i < task_list.size(); i++) {
                line = QString::number(task_list[i].phase) + "," + task_list[i].name + "," + QString::number(task_list[i].isComplete) + "\n";
                out << line;
            }
            file.close();
            return true;
        } else {
            return false;
        }
    }

}


void TSModel::completeTask()
{
    qDebug() << "Entering completeTask";
    setData(QModelIndex(), true, TaskCompleteRole);
    //Assigning it here too because the actual data set and model interface are decoupled.
    task_list[currentTaskIndex].isComplete = true;
    bool incPhase = true;
    for (int i = 0; i < task_list.size(); i++) {
        if (task_list[i].phase == currentPhase && task_list[i].isComplete == false) {
            incPhase = false;
            //qDebug() << "incPhase false.";
        }
    }
    if (incPhase && currentPhase < highestPhase) {
        currentPhase++;
        //qDebug() << "Phase incremented.";
    } else if (incPhase && currentPhase >= highestPhase) {
        isJobComplete = true;
        //qDebug() << "This job is done";
    }


    QString msg = "Task " + task_list[currentTaskIndex].name + " Completed.";
    if (incPhase) {
        msg = msg + " Entering phase " + QString::number(currentPhase);
    }
    qDebug() << msg;

    if (saveJob()) {
        qDebug() << "Job saved to file";
    } else {
        qDebug() << "Error, job could not be saved";
    }
}

void TSModel::changeTask()
{
    qDebug() << "Entering changeTask()";
    //qDebug() << "Phase: " +  QString::number(currentPhase) + " Changing task...";
    bool phaseComplete = true;
    int phaseStartIndex = -1, phaseEndIndex = -1;
    for (int i = 0; i < task_list.size(); i++) {
        //Start by checking if the current phase is complete and something is messed up.
        if (task_list[i].phase == currentPhase && task_list[i].isComplete == false && phaseComplete) {
            phaseComplete = false;
            //qDebug() << "Change Task: Phase Incomplete";
            //qDebug() << "Phase: " + QString::number(task_list[i].phase) + ", Task: " + task_list[i].name + ", Completion: " + QString::number(task_list[i].isComplete);
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

    //qDebug() << "PhaseStartIndex: " + QString::number(phaseStartIndex);
    //qDebug() << "PhaseEndIndex: " + QString::number(phaseEndIndex);

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
    qDebug() << "Entering SortTaskVector()";
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
    qDebug() << "Entering PrintTaskVector()";
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






