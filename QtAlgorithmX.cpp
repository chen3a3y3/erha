#include "QtAlgorithmX.h"
#include <qdesktopwidget.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <future>
#ifdef AC2ME
#include "solver.h"
#include "viewer.h"
#include "input_process.h"
#else
#include "solver_yy.h"
#include "viewer.h"
#include "input_process.h"
#endif // AC2ME

void solve(QtAlgorithmX *qt, shared_ptr<vector<vector<vector<int>>>> test, 
	shared_ptr<Input> input, shared_ptr<Viewer> viewer) {
	auto beginTime = clock();
	int total_result = 0;
	for (int i = 0; i < test->size(); i++) {
		auto &single = (*test)[i];
		Solver S(single.size(), single[0].size(), input->row2positions[i++], viewer.get(), input.get());
		S.show_details = qt->ui.detailBox->isChecked();
		S.qt = qt;
		vector<vector<int>> result = S.solve(single);
		total_result += result.size();
	}
	auto endTime = clock();
	qt->onFinished(total_result);
	QString s = QString::number(endTime - beginTime);
	qt->ui.t2Label->setText("Time to find all solutions: " + s + "ms");
}

void QtAlgorithmX::onFinished(int num_of_sols) {
	auto s = QString::number(num_of_sols);
	emit answerGot("Total number of solutions: " + s);
}

void QtAlgorithmX::onDetailChecked(bool checked) {
	if (checked) ui.singleBox->setChecked(false);
}

void QtAlgorithmX::onSingleChecked(bool checked) {
	if (checked) ui.detailBox->setChecked(false);
}

QtAlgorithmX::QtAlgorithmX(QWidget *parent)
	: QMainWindow(parent)
{
	QRect rec = QApplication::desktop()->screenGeometry();
	desktop_height = rec.height();
	desktop_width = rec.width();
	
	
	ui.setupUi(this);
	this->setGeometry(QRect(desktop_width / 4, desktop_height / 4,
		desktop_width / 2, desktop_height / 2));

	QPixmap bkgnd("..\\QtAlgorithmX\\q.jpg");
	bkgnd = bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio);
	QPalette palette;
	palette.setBrush(QPalette::Background, bkgnd);
	this->setPalette(palette);
//	auto geo = ui.centralWidget->geometry();
//	ui.gridLayout->setGeometry(ui.centralWidget->geometry());
}

void QtAlgorithmX::onButtonClicked()
{
	auto fileName = QFileDialog::getOpenFileName(this,
		tr("Open File"), "/", tr("Text files (*.txt)"));
	emit fileSelected(fileName);
}

void QtAlgorithmX::on_stopButton_clicked() {
	requestedStop = true;
}

void QtAlgorithmX::on_startButton_clicked() {
	requestedStop = false;
	beginTime = clock();
	std::ifstream infile(selected_file);
	if (!infile.good()) return;
	auto test = make_shared<vector<vector<vector<int>>>>();
	auto input = make_shared<Input>();
	if (!input->input_process(selected_file, *test, ui.frBox->isChecked())) return;
	int num = input->total_tile_number;
	int c = input->board->right + 1;
	int r = input->board->down + 1;

	auto viewer = make_shared<Viewer>(c * desktop_height / 20, r * desktop_height / 20);
	viewer->init(num, input->board);

	auto th = std::thread(solve, this, test, input, viewer);
	th.detach();
//	int total_result = 0;
//#ifdef AC2ME
//	int i = 0;
//	for (auto &single: test) {
//		Solver S = Solver(single.size(), single[0].size(), input.row2positions[i++], &viewer, &input);
//		S.show_details = ui.detailBox->isChecked();
//		S.qt = this;
//		vector<vector<int>> result = S.solve(single);
//		total_result += result.size();
//	} 
//	
//#else

//	int i = 0;
//	for (const auto &single : test) {
//		yy::Solver solver;
//		solver.show_details = ui.detailBox->isChecked();
//		solver.init(single, input.row2positions[i++], &viewer);
//		solver.solve(1000000);
//
//		const auto &sol = solver.getSols();
//		total_result += sol.size();
//	}
//	endTime = clock();
//	QString s = QString::number(endTime - beginTime);
//	this->ui.t2Label->setText("Time to find all solutions: " + s + "ms");
//
//	s = QString::number(total_result);
//	emit answerGot("Total number of solutions: " + s);
//	
//#endif
}

void QtAlgorithmX::onFileSelected(QString qs) {
	selected_file = qs.toLocal8Bit().constData();
}
