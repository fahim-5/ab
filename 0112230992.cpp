/*
Number of Team members: 5
ID1: 0112230992 - MD. Abdullah
ID2: 0112230469 - Jakia Tunnesa Rifa
ID3: 0112230420 - Nuzhat Tasnim Netu
ID4: 0112230230 - Marufa Akter
ID5: 0112230846 - Tasnim Jahan Rimvy
*/

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Process {
    string id;
    int arrival;
    int burst;
    int remaining;
    int priority;
    int originalPriority;

    bool isIO;
    int ioAfter;
    int ioDuration;

    bool ioDone = false;
    bool inIO = false;
    int ioCompleteTime = -1;

    int waitingCounter = 0;

    int completion = 0;
    int turnaround = 0;
    int waiting = 0;
    int response = -1;

    bool added = false;
};

bool compareProcess(Process* a, Process* b) {
    if (a->priority == b->priority)
        return a->arrival < b->arrival;

    return a->priority < b->priority;
}

int main() {

    vector<Process> processes = {
        {"P1", 0, 10, 10, 3, 3, false, 0, 0},
        {"P2", 2, 6, 6, 2, 2, true, 3, 2},
        {"P3", 4, 8, 8, 4, 4, false, 0, 0},
        {"P4", 5, 4, 4, 3, 3, true, 2, 3},
        {"P5", 7, 5, 5, 1, 1, false, 0, 0}
    };

    vector<Process*> readyQueue;
    vector<string> gantt;

    int time = 0;
    int completed = 0;
    int n = processes.size();
    int quantum = 4;

    while (completed < n) {

        
        for (auto &p : processes) {
            if (!p.added && p.arrival <= time && !p.inIO) {
                readyQueue.push_back(&p);
                p.added = true;
            }
        }

        for (auto &p : processes) {
            if (p.inIO && p.ioCompleteTime <= time) {
                p.inIO = false;
                p.priority = max(1, p.priority - 2);

                readyQueue.push_back(&p);
            }
        }

     
        for (auto *p : readyQueue) {
            p->waitingCounter++;

            if (p->waitingCounter == 3) {
                p->priority = max(1, p->priority - 1);
                p->waitingCounter = 0;
            }
        }

        sort(readyQueue.begin(), readyQueue.end(), compareProcess);

        if (readyQueue.empty()) {
            time++;
            continue;
        }

        Process *current = readyQueue.front();
        readyQueue.erase(readyQueue.begin());

        current->waitingCounter = 0;

        if (current->response == -1)
            current->response = time - current->arrival;

        int runTime = min(quantum, current->remaining);

        if (current->isIO && !current->ioDone) {

            int executedBefore = current->burst - current->remaining;

            if (executedBefore < current->ioAfter &&
                executedBefore + runTime > current->ioAfter) {

                runTime = current->ioAfter - executedBefore;
            }
        }

        int start = time;
        int end = time + runTime;

        gantt.push_back(current->id + "[" + to_string(start) + "-" + to_string(end) + "]");

        time += runTime;
        current->remaining -= runTime;

        for (auto &p : processes) {
            if (!p.added && p.arrival <= time && !p.inIO) {
                readyQueue.push_back(&p);
                p.added = true;
            }
        }

        if (current->isIO && !current->ioDone) {

            int executed = current->burst - current->remaining;

            if (executed == current->ioAfter) {

                current->ioDone = true;
                current->inIO = true;
                current->ioCompleteTime = time + current->ioDuration;

                continue;
            }
        }

        if (current->remaining == 0) {

            current->completion = time;
            current->turnaround = current->completion - current->arrival;
            current->waiting = current->turnaround - current->burst;

            completed++;
        }
        else {
            readyQueue.push_back(current);
        }
    }

    cout << "\n========== GANTT CHART ==========\n";

    for (string g : gantt)
        cout << g << " | ";

    cout << endl;

    cout << "\n============= PROCESS TABLE =============\n";

    cout << left
         << setw(10) << "Process"
         << setw(10) << "WT"
         << setw(10) << "TAT"
         << setw(10) << "RT"
         << endl;

    double totalWT = 0;
    double totalTAT = 0;

    for (auto &p : processes) {

        totalWT += p.waiting;
        totalTAT += p.turnaround;

        cout << left
             << setw(10) << p.id
             << setw(10) << p.waiting
             << setw(10) << p.turnaround
             << setw(10) << p.response
             << endl;
    }

    cout << "\nAverage Waiting Time = "
         << fixed << setprecision(2)
         << totalWT / n << endl;

    cout << "Average Turnaround Time = "
         << fixed << setprecision(2)
         << totalTAT / n << endl;

    return 0;
}