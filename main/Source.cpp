#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <thread>
#include <Windows.h>
#include <chrono>
#include <atomic>

class Plot
{
public:
	Plot() {}
	void plot(std::vector<double> &X, std::vector<double> &Y,int n_x=10,int n_y=4)
	{
		gridX = n_x;
		gridY = n_y;
		creatCoordinateSystem();  // Инициализация осей координат
		setGrid(n_x, n_y);		  // Инициализация координатной сетки
		Construction();			  // Рендер осей координат и масштабной сетки
		plot_run(X, Y);

		std::thread plot([&]()
		{
			sf::RenderWindow window(sf::VideoMode(width, height), "Graph of function");
			window.setVerticalSyncEnabled(true);
			sf::Event event;
			while (window.isOpen())
			{
				while (window.pollEvent(event))
				{
					if (event.type == sf::Event::Closed)
						window.close();
				}

				// Отрисовка
				if (!pause.load())
				{
					window.clear(sf::Color(232, 232, 232, 255));
					window.draw(System);
					window.draw(Grid);
					for (int n = 0; n < 3; n++)
						window.draw(Title[n]);
					for (int n = 0; n < 2; n++)
					{
						for (int m = 0; m < SignaturesGrid[n].size(); m++)
							window.draw(SignaturesGrid[n][m]);
					}
					window.draw(RenderFunct);
					window.display();
				}
			}
		});
		plot.detach();
	}
	void plot_pause(bool voll, int millisec=25)
	{
		Sleep(millisec);
		pause.store(voll);
		ScaleFunction.clear();
		RenderFunct.clear();
		source.clear();
		Thinning_X.clear();
		Thinning_Y.clear();
	}
	void plot_run(std::vector<double> &X, std::vector<double> &Y)
	{
		source.resize(2);
		source[0] = X;
		source[1] = Y;
		ThinningFucnt(source[0], source[1]); // Прореживание функции
		ScaleFunct(Thinning_X, Thinning_Y);  // Масштабирование принятой функции для окна вывода
		RenderFunction();					 // Рендеринг функции
		setSignaturesGrid(gridX, gridY);     // Раставить подписи координатной сетки
		pause.store(false);
	}

	void setTitle(std::wstring Text)
	{
		std::wstring T = Text;
		setTitle("Title", T);
	}
	void setLabel_X(std::wstring Text)
	{
		std::wstring T = Text;
		setTitle("X_Label", T);
	}
	void setLabel_Y(std::wstring Text)
	{
		std::wstring T = Text;
		setTitle("Y_Label", T);
	}

private:
	//Свойства
	int width = 600;  //Длина окна в пиксилях
	int height = 400; // Высота окна в пикселях
	std::vector<std::vector<int>> XY; // Массив точек для построение системы координат
	std::vector<std::vector<int>> GridXY; // Массив точек для построения координатной сетки
	sf::VertexArray System; // Рендорит системо координат
	sf::VertexArray Grid;	// Рендерит координатную сетку
	std::vector<std::vector<int>> ScaleFunction; // Смаштабированная функция
	sf::VertexArray RenderFunct; // Рендерит график функции
	sf::Font font;     // Инициализация шрифта
	sf::Text Title[3]; // Названия осей координат и графика
	std::vector<sf::Text> SignaturesGrid[2]; // Подписи для сетки
	std::vector<std::vector<double>> source; // Исходные данные функии 
	std::vector<double> Thinning_X; // Прореженный ряд X
	std::vector<double> Thinning_Y; // Прореженный ряд Y
	int f_0_Y; // Указывает координаты на положение нуля оси Y графика функции
	int f_0_X; // Указывает координаты на положение нуля оси X графика функции
	int step[2] = {0,0}; // Шаг для сетки осей
	int gridX;
	int gridY;
	std::atomic<bool> pause=false;

	//Местоды
	void ThinningFucnt(std::vector<double> &X, std::vector<double> &Y)  // Прореживание функции и запись исходной в буфер
	{
		if (source[0].size() > (width*2) && source[0].size() < (width * 20))
		{
			int k = (int)round(source[0].size() / ((double)width * 2));
			for (int n = 0; n < source[0].size(); n += k)
			{
				Thinning_X.push_back(source[0][n]);
				Thinning_Y.push_back(source[1][n]);
			}
		}
		else
		{
			Thinning_X = source[0];
			Thinning_Y = source[1];
		}
	}
	void calcGridfuncziro()
	{

	}
	void creatCoordinateSystem()  // Построение осей координат
	{
		double Proportion = (double)width / (double)height;
		XY.resize(2, std::vector<int>(4));
		// Задаем ось Y;
		/*"X"*/XY[0][0] = (int)round(width* 0.0666666666666666);			    XY[0][1] = (int)round(width*0.0666666666666666);
		/*"Y"*/XY[1][0] = (int)round(height*0.0666666666666666*Proportion);		XY[1][1] = (int)round(height*(1 - 0.055*Proportion));
		// Задаем ось X
		/*"X"*/XY[0][2] = (int)round(width*0.055);									XY[0][3] = (int)round(width*(1 - 0.0666666666666666));
		/*"Y"*/XY[1][2] = (int)round(height*(1 - 0.0666666666666666*Proportion));	XY[1][3] = (int)round(height*(1 - 0.0666666666666666*Proportion));

	}
	void Construction(bool Grid=true) // Рендер осей координат и координатной сетки
	{
		System.setPrimitiveType(sf::PrimitiveType::Lines);
		System.resize(XY[0].size() * 2);
		for (int n = 0; n < XY[0].size(); n++)
			System.append(sf::Vertex(sf::Vector2f(XY[0][n], XY[1][n]), sf::Color(0, 0, 0, 255)));

		if (Grid == true)
		{
			this->Grid.setPrimitiveType(sf::PrimitiveType::Lines);
			this->Grid.resize(GridXY[0].size() * 4);

			for (int n = 0; n < GridXY[0].size(); n++)
			{
				for (int m = 0; m < 4; m += 2)
					this->Grid.append(sf::Vertex(sf::Vector2f(GridXY[m][n], GridXY[m + 1][n]), sf::Color(0, 0, 255, 50)));
			}
		}
	}
	void setGrid(int n_x, int n_y) // Метод создает координатную сетку
	{

		GridXY.resize(4, std::vector <int>(n_x + n_y));
		// Задаем шаг координатной сетки
		// Для оси Y
		step[1] = (int)trunc(((float)XY[1][2] - (float)(f_0_Y + XY[1][0])) / (float)n_y);
		// Для оси X
		step[0] = (int)trunc(((float)XY[0][3] - (float)(f_0_X + XY[0][0])) / (float)n_x);
		// Создание сетки для оси Y
		for (int n = 1; n <= n_y; n++) // Задает начальные точки для сетки Y
		{
			/*"X"*/GridXY[0][n - 1] = XY[0][0]; // Ось икс конст
			/*"Y"*/GridXY[1][n - 1] = XY[1][2] - step[1] * n; // Вычисляем только игрек
		}
		for (int n = 1; n <= n_y; n++) // Задает конечные точки для сетки Y
		{
			/*"X"*/GridXY[2][n - 1] = XY[0][3]; // Ось икс конст
			/*"Y"*/GridXY[3][n - 1] = XY[1][2]- step[1] * n; // Вычисляем только игрек
		}
		// Создание сетки для оси Х
		for (int n = n_y; n < n_x + n_y; n++) // Задает начальные точки для сетки X
		{
			/*"X"*/GridXY[0][n] = step[0] * (n - (n_y - 1)) + XY[0][0]; // Вычисляем только икс
			/*"Y"*/GridXY[1][n] = XY[1][2]; // Ось игрек конст
		}
		for (int n = n_y; n < n_x + n_y; n++) // Задает начальные точки для сетки X
		{
			/*"X"*/GridXY[2][n] = step[0] * (n - (n_y - 1)) + XY[0][0]; // Вычисляем только икс
			/*"Y"*/GridXY[3][n] = XY[1][0]; // Ось игрек конст
		}
	}
	void setSignaturesGrid(int n_x, int n_y, bool m=false) // Метод подписывает координатную сетку
	{
		for (int i = 0; i < 2; i++) // Цикл заполняет массивы подписей (i=0 для X, i=1 для Y)
		{
			int Coordinate; // Флаг, указывает заполняющию координату(а число в флаге показывает на сколько частей разбивать ось)
			int a; // Переменная помогает перемещаться по массиву сетки координат
			int b;
			switch (i) // Поднимает флаг указателя
			{
				case 0:
					Coordinate = n_x;
					a = n_y;
					b = 0;
				break;
				case 1:
					Coordinate = n_y;
					a = 0;
					b = 23;
				break;
			}
			SignaturesGrid[i].resize(Coordinate + 3); // Массив с подписями
			double step = (*std::max_element(source[i].begin(), source[i].end()) - *std::min_element(source[i].begin(), source[i].end()))/ Coordinate;
			double tens_degre = round(log10(step));
			int whole = (int)pow(10, abs(tens_degre)); // Возведем десятку в её степень чтобы после выделить целую часть чисел на подписях

			for (int n = 0; n < SignaturesGrid[i].size(); n++)
			{
				SignaturesGrid[i][n].setFont(font);  // Устанавливаем шрифт
				SignaturesGrid[i][n].setCharacterSize(11); // Устанавливаем размер шрифта
				SignaturesGrid[i][n].setFillColor(sf::Color(64, 64, 64, 255));

				if (n < Coordinate)
				{
					if (tens_degre < 0) // Если числа на оси меньше 1цы
					{
						double numbeer = (*std::min_element(source[i].begin(), source[i].end()) + step * (n + 1)) * whole;
						std::string Intermediate = std::to_string(numbeer);
						std::string volue;
						for (int j = 0; j < 4; j++)
							volue.push_back(Intermediate[j]);

						SignaturesGrid[i][n].setString(volue);
						SignaturesGrid[i][n].setPosition(GridXY[0][n + a], GridXY[1][n + a]);
					}
					if(tens_degre >= 0 && tens_degre <= 3)
					{
						double numbeer = (*std::min_element(source[i].begin(), source[i].end()) + step * (n + 1));
						std::string Intermediate = std::to_string(numbeer);
						std::string volue;
						for (int j = 0; j < 4; j++)
							volue.push_back(Intermediate[j]);

						SignaturesGrid[i][n].setString(volue);
						SignaturesGrid[i][n].setPosition(GridXY[0][n + a]-b, GridXY[1][n + a]);
					}
					if (tens_degre > 3)
					{
						double numbeer = (*std::min_element(source[i].begin(), source[i].end()) + step * (n + 1)) / whole;
						std::string Intermediate = std::to_string(numbeer);
						std::string volue;
						for (int j = 0; j < 4; j++)
							volue.push_back(Intermediate[j]);

						SignaturesGrid[i][n].setString(volue);
						SignaturesGrid[i][n].setPosition(GridXY[0][n + a], GridXY[1][n + a]);
					}
				}
				else
				{
					if (n == Coordinate)
					{
						SignaturesGrid[i][n].setString("x10");
						if (i == 0)
							SignaturesGrid[i][n].setPosition(round((double)width*0.88333333), round((double)height*0.95));
						else
							SignaturesGrid[i][n].setPosition(round((double)width*0.008333333), round((double)height*0.05));
					}
					if(n == Coordinate+1)
					{
						std::string Intermediate = std::to_string(tens_degre);
						std::string volue;
						for (int j = 0; j < 10000000; j++)
						{
							if (Intermediate[j] != ',')
								volue.push_back(Intermediate[j]);
							else
								break;
						}
						SignaturesGrid[i][n].setString(volue);
						if (i == 0)
							SignaturesGrid[i][n].setPosition(round((double)width*0.913333333), round((double)height*0.93));
						else
							SignaturesGrid[i][n].setPosition(round((double)width*0.038333333), round((double)height*0.03));
					}
					if(n == Coordinate + 2)
					{
						if (i == 0)
						{
							double numbeer = *std::min_element(source[i].begin(), source[i].end());
							std::string Intermediate = std::to_string(numbeer);
							std::string volue;
							for (int j = 0; j < 4; j++)
								volue.push_back(Intermediate[j]);
							SignaturesGrid[i][n].setString(volue);
							SignaturesGrid[i][n].setPosition(round((double)width*0.06666666),round((double)height*0.9));

						}
						else
						{
							double numbeer = *std::min_element(source[i].begin(), source[i].end());
							std::string Intermediate = std::to_string(numbeer);
							std::string volue;
							for (int j = 0; j < 4; j++)
								volue.push_back(Intermediate[j]);
							SignaturesGrid[i][n].setString(volue);
							SignaturesGrid[i][n].setPosition(round((double)width*0.02666666), round((double)height*0.8625));
						}
					}
				}
			}

		}

	}
	void ScaleFunct(std::vector<double> &X, std::vector<double> &Y)
	{
		// Масштабирование для Y
		ScaleFunction.resize(2, std::vector<int>(Y.size()));
		double max_Y = *std::max_element(Y.begin(), Y.end());
		double min_Y = *std::min_element(Y.begin(), Y.end());
		double K_Y = abs((max_Y - min_Y) / ((double)XY[1][0] - (double)XY[1][2]));
		for (int n = 0; n < Y.size(); n++)
			ScaleFunction[1][n] = (int)round(Y[n] / K_Y);
		if ((int)*std::min_element(ScaleFunction[1].begin(), ScaleFunction[1].end()) < 0)
		{
			f_0_Y = XY[1][2] - (int)abs(*std::min_element(ScaleFunction[1].begin(), ScaleFunction[1].end()));
			for (int n = 0; n < Y.size(); n++)
				ScaleFunction[1][n] = f_0_Y - ScaleFunction[1][n];
		}
		else
		{
			for (int n = 0; n < X.size(); n++)
				ScaleFunction[1][n] = XY[1][2] - ScaleFunction[1][n];
		}
		// Масштабирование для X
		double max_X = *std::max_element(X.begin(), X.end());
		double min_X = *std::min_element(X.begin(), X.end());
		double K_X = abs((max_X - min_X) / ((double)XY[0][3] - (double)XY[0][1]));
		for (int n = 0; n < Y.size(); n++)
			ScaleFunction[0][n] = (int)round(X[n] / K_X);
		if ((int)*std::min_element(ScaleFunction[0].begin(), ScaleFunction[0].end()) < 0)
		{
			int Correction = (int)abs(*std::min_element(ScaleFunction[0].begin(), ScaleFunction[0].end()) - XY[0][1]);
			f_0_X = Correction;
			for (int n = 0; n < Y.size(); n++)
				ScaleFunction[0][n] = ScaleFunction[0][n] + Correction;
		}
		else
		{
			for (int n = 0; n < X.size(); n++)
				ScaleFunction[0][n] = XY[0][1] + ScaleFunction[0][n];
		}
	}
	void RenderFunction()
	{
		RenderFunct.setPrimitiveType(sf::PrimitiveType::Lines);
		double S = round((double)ScaleFunction[0].size()*(2 - 2 / ScaleFunction[0].size()));
		RenderFunct.resize(S);
		for (int n = 0; n < ScaleFunction[0].size() - 1; n++)
		{
			for (int m = 0; m < 2; m++)
				RenderFunct.append(sf::Vertex(sf::Vector2f(ScaleFunction[0][n + m], ScaleFunction[1][n + m]), sf::Color(255, 0, 0, 255)));
		}

	}
	int  setTitle(std::string Label, std::wstring &Text)
	{
		// Создание текста
		if (!font.loadFromFile("times.ttf"))
		{
			return EXIT_FAILURE;
		}

		if (Label == "X_Label")
		{
			Title[0].setFont(font);
			Title[0].setString(Text);
			Title[0].setCharacterSize(16);
			Title[0].setFillColor(sf::Color(64, 64, 64, 255));
			int width_X_Label = Title[0].getLocalBounds().width;
			Title[0].setPosition((width / 2) - ((int)round(width_X_Label / 2)), height - 25);
		}
		if (Label == "Y_Label")
		{
			Title[1].setFont(font);
			Title[1].setString(Text);
			Title[1].setCharacterSize(16);
			Title[1].setFillColor(sf::Color(64, 64, 64, 255));
			Title[1].rotate(270);
			int width_Y_Label = Title[1].getLocalBounds().width;
			Title[1].setPosition(0, (height / 2) + ((int)round(width_Y_Label / 2)));
		}
		if (Label == "Title")
		{
			Title[2].setFont(font);
			Title[2].setString(Text);
			Title[2].setCharacterSize(16);
			Title[2].setFillColor(sf::Color(64, 64, 64, 255));
			int width_Titles = Title[2].getLocalBounds().width;
			Title[2].setPosition((width / 2) - ((int)round(width_Titles / 2)), 5);
		}
		return NULL;
	}
};

int main()
{
	setlocale(LC_ALL, "RUSSIAN");
	int Fs = 10e3;
	int F = 100;
	double T = (double)1 / (double)Fs;
	double pi = 3.1415926535897932384626433832795;
	std::vector<double> t;
	for (double n = 0; n < 0.025; n += T)
	{
		t.push_back(n);
	}
	std::vector<double> S(t.size());
	for (int n = 0; n < t.size(); n++)
	{
		S[n] = sin(2 * pi*F*t[n])*sin(2 * pi*10*F*t[n]);
		//S.push_back(pow(t[n],2)+2*t[n]-50);
	}
	Plot Create;
	Create.setTitle(L"График амплитудной модуляции");
	Create.setLabel_X(L"Секунды");
	Create.setLabel_Y(L"Значение отсчетов");

	Create.plot(t, S);
    double k = T;
	while (true)
	{
		Create.plot_pause(true);
		k++;
		for (int n = 0; n < t.size(); n++)
			S[n] = sin(2 * pi*F*t[n]+k)*sin(2 * pi * 10 * F*t[n]+k);;
		
		Create.plot_run(t, S);
	}

	system("Pause");
	return 0;
}