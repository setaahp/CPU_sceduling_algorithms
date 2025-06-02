#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compareArrival(const void *a, const void *b) {
    Process *p1 = (Process *)a;
    Process *p2 = (Process *)b;
    return p1->arrivalTime - p2->arrivalTime;
}

Metrics calculate_answer(Process proc[], int n) {
    float Turnaround = 0, Waiting = 0, Response = 0;
    for (int i = 0; i < n; i++) {
        int curturnaround = proc[i].completionTime - proc[i].arrivalTime;
        int curwaiting = curturnaround - proc[i].burstTime;
        int curresponse = proc[i].startTime - proc[i].arrivalTime;

        Turnaround += curturnaround;
        Waiting += curwaiting;
        Response += curresponse;
    }
    Metrics m;
    m.avgTurnaround = Turnaround / n;
    m.avgWaiting = Waiting / n;
    m.avgResponse = Response / n;
    return m;
}

Metrics fcfs_metrics(Process proc[], int n) {
    Process temp[n];
    memcpy(temp, proc, sizeof(Process) * n);
    qsort(temp, n, sizeof(Process), compareArrival);

    int time = 0;
    for (int i = 0; i < n; i++) {
        if (time < temp[i].arrivalTime) time = temp[i].arrivalTime;
        temp[i].startTime = time;
        time += temp[i].burstTime;
        temp[i].completionTime = time;
    }
    return calculate_answer(temp, n);
}

Metrics sjf_metrics(Process proc[], int n) {
    Process temp[n];
    memcpy(temp, proc, sizeof(Process) * n);

    int completed = 0, time = 0;
    int isCompleted[n];
    memset(isCompleted, 0, sizeof(isCompleted));

    while (completed < n) {
        int idx = -1;
        int minBurst = 1e9;
        // shortest job available now
        for (int i = 0; i < n; i++) {
            if (!isCompleted[i] && temp[i].arrivalTime <= time && temp[i].burstTime < minBurst) {
                minBurst = temp[i].burstTime;
                idx = i;
            }
        }
        if (idx == -1)
            time++; // no process ready so go on
        else {
            temp[idx].startTime = time;
            time += temp[idx].burstTime;
            temp[idx].completionTime = time;
            isCompleted[idx] = 1;
            completed++;
        }
    }
    return calculate_answer(temp, n);
}

Metrics rr_metrics(Process proc[], int n, int timeQuantum) {
    Process temp[n];
    memcpy(temp, proc, sizeof(Process) * n);
    for (int i = 0; i < n; i++) {
        temp[i].remainingTime = temp[i].burstTime;
        temp[i].startTime = -1; // determine first response time
    }

    int time = 0 , completed = 0 , front = 0, rear = 0;
    int queue[1000] , visited[n];
    memset(visited, 0, sizeof(visited));

    while (completed < n) {
        // add processes that just arrived to the queue
        for (int i = 0; i < n; i++) {
            if (!visited[i] && temp[i].arrivalTime <= time) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }

        if (front == rear) {
            time++; // no process to run so go on
            continue;
        }

        int idx = queue[front++]; // remove processes from queue
        if (temp[idx].startTime == -1)
            temp[idx].startTime = time; // first response time

        int execTime = timeQuantum < temp[idx].remainingTime ? timeQuantum : temp[idx].remainingTime;
        temp[idx].remainingTime -= execTime;
        time += execTime;
        // add new processes to queue
        for (int i = 0; i < n; i++) {
            if (!visited[i] && temp[i].arrivalTime <= time) {
                queue[rear++] = i;
                visited[i] = 1;
            }
        }

        if (temp[idx].remainingTime > 0)
            queue[rear++] = idx; // add processes to queue if not finished
        else {
            temp[idx].completionTime = time;
            completed++;
        }
    }

    return calculate_answer(temp, n);
}
