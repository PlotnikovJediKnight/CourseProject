#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

typedef struct {
	int flow;
	int capacity;
} netEdge;

#define MAX_ARR_LENGTH 100
netEdge network[MAX_ARR_LENGTH][MAX_ARR_LENGTH];
int		 resNet[MAX_ARR_LENGTH][MAX_ARR_LENGTH];

const int MAX_VERTEXES = 20;

void initializeNetworkWithZeros(void) {
	for (int i = 0; i < MAX_ARR_LENGTH; i++)
		for (int j = 0; j < MAX_ARR_LENGTH; j++) {
			network[i][j].flow = 0;
			network[i][j].capacity = 0;
		}
}

void initializeResidualNetworkWithZeros(void) {
	for (int i = 0; i < MAX_ARR_LENGTH; i++)
		for (int j = 0; j < MAX_ARR_LENGTH; j++)
			resNet[i][j] = 0;
}

int SINK_VERTEX = 0;
int N = 0;
int M = 0;

void initializeNetworkWithData(void) {
	for (int i = 0; i < M; i++) {
		int u = 1, v = 1, c = 0;
		scanf("%d %d %d", &u, &v, &c);

		u--;
		v--;

		network[u][v].capacity = c;
	}
}

void handleAntiparallelEdges(void) {
	for (int u = 0; u < N; u++) {
		for (int v = 0; v < N; v++) {
			if (network[u][v].capacity > 0 && 
				network[v][u].capacity > 0) {
				int c = network[v][u].capacity;
				network[v][u].capacity = 0;

				network[v][N].capacity = c;
				network[N][u].capacity = c;

				N++;
			}
		}
	}
}

void updateResidualNetwork(void) {
	for (int u = 0; u < N; u++) {
		for (int v = 0; v < N; v++) {
			int cuv = network[u][v].capacity;
			if (cuv > 0) {
				resNet[u][v] = cuv - network[u][v].flow;
			}
			else {
				resNet[u][v] = network[v][u].flow;
			}
		}
	}
}

short int   visited[MAX_ARR_LENGTH];
int		   distance[MAX_ARR_LENGTH];
int		   shortest[MAX_ARR_LENGTH];
#define INFINITY 9999999

int DijkstraAlgorithm(int source, int sink) {
	for (int i = 0; i < N; i++) {
		distance[i] = INFINITY;
		 visited[i] = 0;
		shortest[i] = -1;
	}

	distance[source] = 0;

	for (int i = 0; i < N; i++) {
		int min = INFINITY;
		int u = 0;

		for (int j = 0; j < N; j++)
			if (!visited[j] && distance[j] < min) {
				min = distance[j];
				u = j;
			}

		visited[u] = 1;

		for (int j = 0; j < N; j++)
			if (!visited[j] && resNet[u][j] > 0 && distance[u] + 1 < distance[j]) {
				distance[j] = distance[u] + 1;
				shortest[j] = u;
			}
	}

	if (distance[sink] == INFINITY) {
		return 0;
	}
	else {
		int minResCap = INFINITY;
		int curr = sink, prev = sink;

		while (shortest[curr] != -1) {
			curr = shortest[curr];
			minResCap = (minResCap > resNet[curr][prev]) ? resNet[curr][prev] : minResCap;
			prev = curr;
		}

		return minResCap;
	}
}

void updateNetwork(int minResCap) {
	int curr = SINK_VERTEX, prev = SINK_VERTEX;

	while (shortest[curr] != -1) {
		curr = shortest[curr];

		if (network[curr][prev].capacity > 0)
			network[curr][prev].flow += minResCap;
		else
			network[prev][curr].flow -= minResCap;

		prev = curr;
	}
}

int main(void) {

	initializeNetworkWithZeros();
	initializeResidualNetworkWithZeros();

	scanf("%d %d", &N, &M);
	SINK_VERTEX = N - 1;
	initializeNetworkWithData();

	handleAntiparallelEdges();
	updateResidualNetwork();

	while (1) {
		int minResCap = DijkstraAlgorithm(0, SINK_VERTEX);
		if (minResCap == 0) break;
		updateNetwork(minResCap);
		updateResidualNetwork();
	}

	int MFLOW = 0;
	for (int i = 0; i < N; i++)
		MFLOW += network[i][SINK_VERTEX].flow;

	printf("%d", MFLOW);

	return 0;
}