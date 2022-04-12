#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;
#define MAX 500
#define port 5200

struct proceso
{
	int clientSocket;
	string text;
};

void *SendToSocket(void *stru)
{
	struct proceso *p1 = (struct proceso *)stru;
	// proceso p1=*((proceso*)(&stru));
	char buff[MAX];
	char input[MAX];
	string s = p1->text;
	int n = s.size();
	for (int i = 0; i < n; i++)
	{
		input[i] = s[i];
	}
	// Enviar a Server
	send(p1->clientSocket, input, strlen(input) + 1, 0);
	pthread_exit(NULL);
}

void *manual(void *clientSocket)
{
	int socket = *((int *)(&clientSocket));
	ifstream file("manual.txt");
	string str;

	pthread_t threads[10];
	int i = 0;

	sleep(2);
	while (getline(file, str))
	{
		i += 1;
		try
		{
			int frecuencia = rand() % 3 + 8;
			string b = str.substr(0, str.find(","));
			string p = str.substr(str.find(",") + 1, str.find("\n"));
			int burst = stoi(b);
			int prioridad = stoi(p);

			proceso p1;
			p1.clientSocket = socket;
			p1.text = str;

			int ret = pthread_create(&threads[i], NULL, &SendToSocket, &p1);
			if (ret != 0)
			{
				cout << "Error in thread join " << ret << endl;
			}

			cout << burst << " " << prioridad << "\n"
				 << flush;
			sleep(frecuencia);
		}
		catch (...)
		{
			cout << "Se han enviado todos los procesos\n";
		}
	}
	file.close();
	pthread_exit(NULL);
}

void *enviaMensaje(void *cSocket)
{
	int clientSocket = *((int *)(&cSocket));
	char buff[MAX];
	while (true)
	{
		char input[MAX] = {};
		string s;
		// cout << "Client : ";
		getline(cin, s);
		int n = s.size();
		if (n != 0)
		{
			for (int i = 0; i < n; i++)
			{
				input[i] = s[i];
			}
			send(clientSocket, input, strlen(input) + 1, 0);
		}
		if (input[0] == 'b' && input[1] == 'y' && input[2] == 'e')
			break;
	}
	pthread_exit(NULL);
}

void *automatico(void *clientSocket)
{
	int socket = *((int *)(&clientSocket));
	string rango;
	cout << "Ingrese el rango de valores para el burst separados por espacio: ";
	getline(cin, rango);

	string a = rango.substr(0, rango.find(" "));
	string b = rango.substr(rango.find(" ") + 1, rango.find("\n"));
	int rango1 = stoi(a);
	int rango2 = stoi(b);
	cout << rango1 << " " << rango2 << "\n";
	// cin.clear();
	int frecuencia;
	cout << "Ingrese la frecuencia con la que se quiere enviar los procesos: ";
	cin >> frecuencia;

	sleep(2);
	pthread_t threadRecibir;
	if (pthread_create(&threadRecibir, NULL, enviaMensaje, (void *)socket) != 0)
	{
		cout << "Error in thread join " << endl;
	}
	int burst, prioridad;

	pthread_t threads[10];
	int i = 0;
	while (true)
	{
		i += 1;

		burst = rand() % (rango2 - rango1 + 1) + rango1;
		prioridad = 1 + rand() % 5;
		cout << burst << " " << prioridad << "\n"
			 << flush;

		proceso p1;
		p1.clientSocket = socket;
		p1.text = to_string(burst) + "," + to_string(prioridad);

		int ret = pthread_create(&threads[i], NULL, &SendToSocket, &p1);
		if (ret != 0)
		{
			cout << "Error in thread join " << ret << endl;
		}

		sleep(frecuencia);
		pthread_cancel(threads[i]);
	}
	pthread_exit(NULL);
}

void *cliente(void *)
{
	int clientSocket, serverSocket, receiveMsgSize;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket < 0)
	{
		cout << "Creation of client socket failed" << endl;
	}
	struct sockaddr_in serverAddr, clientAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(port);
	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		cout << "Connection Error..." << endl;
	}
	else
	{
		cout << "\t\tConnection Established..." << endl;
	}
	string str;
	int tipo;
	cout << "Seleccione el modo:\n1-Manual\n2-Automatico\n";
	getline(cin, str);
	tipo = stoi(str);
	// cin >> tipo;
	// tipo = 2;

	pthread_t threadModo;
	pthread_t threadRecibir;
	if (tipo == 1)
	{
		if (pthread_create(&threadModo, NULL, manual, (void *)clientSocket) != 0)
		{
			cout << "Error in thread join " << endl;
		}
		if (pthread_create(&threadRecibir, NULL, enviaMensaje, (void *)clientSocket) != 0)
		{
			cout << "Error in thread join " << endl;
		}
	}
	else if (tipo == 2)
	{
		pthread_t threadAuto;
		if (pthread_create(&threadModo, NULL, automatico, (void *)clientSocket) != 0)
		{
			cout << "Error in thread join " << endl;
		}
	}

	while (true)
	{

		char receiveMessage[MAX] = "";
		int rMsgSize = recv(clientSocket, receiveMessage, MAX, 0);
		if (rMsgSize < 0)
		{
			cout << "Packet recieve failed." << endl;
			exit(-1);
		}
		else if (rMsgSize == 0)
		{
			cout << "Server is off." << endl;
			pthread_cancel(threadRecibir);
			pthread_cancel(threadModo);
			exit(-1);
		}

		if (receiveMessage[0] == 'b' && receiveMessage[1] == 'y' && receiveMessage[2] == 'e')
		{
			cout << "\nConnection ended.";
			break;
		}
		cout << "Server: " << receiveMessage << endl;
	}
	close(clientSocket);
	pthread_exit(NULL);
}

int main()
{
	int rc;
	pthread_t threadServer;
	rc = pthread_create(&threadServer, NULL, cliente, NULL);
	if (rc)
	{
		cout << "Error:unable to create thread," << rc << endl;
		exit(-1);
	}

	sleep(2);

	pthread_exit(NULL);
}
