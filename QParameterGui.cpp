/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QButtonGroup>
#include <QRadioButton>

#include "QParameterGui.h"
#include "QCSXCAD_Global.h"

QParameter::QParameter(Parameter* para, QWidget* parent) : QGroupBox(parent)
{
	clPara=para;
	Name=new QLabel("");
	Value=new QLineEdit("");
	connect(Value,&QLineEdit::textEdited,this,&QParameter::Changed);
	
	
	lay = new QGridLayout();
//	lay->addWidget(new QLabel(tr("Name: ")),0,0);
//	lay->addWidget(Name,0,1);

	lay->addWidget(new QLabel(tr("Value: ")),0,0);
	lay->addWidget(Value,0,1);
	
	QPushButton* btn = new QPushButton(QIcon(":/images/failed.png"),QString());
	connect(btn,&QPushButton::clicked,this,&QObject::deleteLater);	
	connect(btn,&QPushButton::clicked,this,&QParameter::DeleteParameter);
	btn->setToolTip(tr("Delete parameter"));	
	lay->addWidget(btn,0,3);
	
	SweepCB = new QCheckBox();
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
	connect(SweepCB,&QCheckBox::checkStateChanged,this,[this](Qt::CheckState s){SweepState(static_cast<int>(s));});
#else
	connect(SweepCB,&QCheckBox::stateChanged,this,&QParameter::SweepState);
#endif
	SweepCB->setToolTip(tr("Sweep this parameter"));
	lay->addWidget(SweepCB,0,2);
	
	setLayout(lay);	
	//setFrameStyle(QFrame::Box);
}

QParameter::~QParameter()
{
}

void QParameter::DeleteParameter()
{
	emit Delete(clPara);
}

void QParameter::SweepState(int state)
{
	clPara->SetSweep(state);
}

void QParameter::Changed()
{
	double val=Value->text().toDouble();
	clPara->SetValue(val);
	emit ParameterChanged();
}

bool QParameter::Edit()
{
	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Edit Constant Parameter"));

	QGridLayout* lay = new QGridLayout();
	
	lay->addWidget(new QLabel(tr("Name: ")),0,0);
	QLineEdit* eName= new QLineEdit(QString::fromUtf8(clPara->GetName().c_str()));
	lay->addWidget(eName,0,1);
	
	lay->addWidget(new QLabel(tr("Value: ")),1,0);
	QLineEdit* eValue=new QLineEdit(QString("%1").arg(clPara->GetValue()));
	lay->addWidget(eValue,1,1);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::accept);
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::reject);
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		if (eName->text().isEmpty())
		{
			QMessageBox::warning(this,tr("Edit Parameter"),tr("Parameter-Name is invalid!"));
			return false;
		}
		clPara->SetName(eName->text().toStdString());
		clPara->SetValue(eValue->text().toDouble());
		Update();
		return true;
	}		
	return false;
}

void QParameter::Update()
{
	//Name->setText(clPara->GetName());
	setTitle(QString::fromUtf8(clPara->GetName().c_str()));
	Value->setText(QString("%1").arg(clPara->GetValue()));
	if (clPara->GetSweep()) SweepCB->setCheckState(Qt::Checked);
	else SweepCB->setCheckState(Qt::Unchecked);
}

QLinearParameter::QLinearParameter(LinearParameter* para, QWidget* parent) : QParameter(para,parent)
{
	slider = new QSlider(Qt::Horizontal);
	lay->addWidget(slider,1,0,1,3);
	Value->setReadOnly(true);
	connect(slider,&QSlider::valueChanged,this,&QLinearParameter::Changed);

	QPushButton* btn = new QPushButton(QIcon(":/images/edit.png"),QString());
	connect(btn,&QPushButton::clicked,this,&QLinearParameter::Edit);	
	lay->addWidget(btn,1,3);
	btn->setToolTip(tr("Edit Parameter"));
}


QLinearParameter::~QLinearParameter()
{
}

bool QLinearParameter::Edit()
{
	LinearParameter* LP = clPara->ToLinear();
	if (LP==nullptr) return false;

	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Edit Linear Parameter"));

	QGridLayout* lay = new QGridLayout();
	
	lay->addWidget(new QLabel(tr("Name: ")),0,0);
	QLineEdit* eName= new QLineEdit(QString::fromUtf8(LP->GetName().c_str()));
	lay->addWidget(eName,0,1);
	
	lay->addWidget(new QLabel(tr("Value: ")),1,0);
	QLineEdit* eValue=new QLineEdit(QString("%1").arg(LP->GetValue()));
	lay->addWidget(eValue,1,1);	
	
	lay->addWidget(new QLabel(tr("Start: ")),2,0);
	QLineEdit* Start=new QLineEdit(QString("%1").arg(LP->GetMin()));
	lay->addWidget(Start,2,1);	
	
	lay->addWidget(new QLabel(tr("Stop: ")),3,0);
	QLineEdit* Stop=new QLineEdit(QString("%1").arg(LP->GetMax()));
	lay->addWidget(Stop,3,1);	
	
	lay->addWidget(new QLabel(tr("Step: ")),4,0);
	QLineEdit* Step=new QLineEdit(QString("%1").arg(LP->GetStep()));
	lay->addWidget(Step,4,1);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::accept);
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::reject);
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		if (eName->text().isEmpty())
		{
			QMessageBox::warning(this,tr("Edit Parameter"),tr("Parameter-Name is invalid!"));
			return false;
		}
		LP->SetName(eName->text().toStdString());
		LP->SetMin(Start->text().toDouble());
		LP->SetMax(Stop->text().toDouble());
		LP->SetStep(Step->text().toDouble());
		LP->SetValue(eValue->text().toDouble());
		Update();
		return true;
	}		
	return false;
}

void QLinearParameter::Update()
{
	QParameter::Update();
	LinearParameter* LP=clPara->ToLinear();
	if (LP==nullptr) return;
	if ((LP->GetStep()>0) && (LP->GetMax()>LP->GetMin()))
	{
		double steps=(LP->GetMax()-LP->GetMin())/LP->GetStep();
		double val=(LP->GetValue()-LP->GetMin())/LP->GetStep();
		slider->setRange(1,(int)steps+1);
		slider->setValue((int)val+1);
	}
	else slider->setRange(1,1);
}

void QLinearParameter::Changed()
{
	LinearParameter* LP=clPara->ToLinear();
	if (LP==nullptr) return;
	double val=LP->GetMin()+LP->GetStep()*(slider->value()-1);
	LP->SetValue(val);
	Value->setText(QString("%1").arg(LP->GetValue()));
	emit ParameterChanged();
}

QParameterSet::QParameterSet(QWidget* parent) : QWidget(parent), ParameterSet()
{
	QGridLayout* grid = new QGridLayout();
	
	ParaLay = new QVBoxLayout();

	QScrollArea* QSA = new QScrollArea();
	QWidget* QW = new QWidget();
	
	QVBoxLayout* QVBL = new QVBoxLayout();
	QVBL->addLayout(ParaLay);
	QVBL->addStretch(1);
	QW->setLayout(QVBL);
	QSA->setWidget(QW);
	QSA->setWidgetResizable(true);
	
	grid->addWidget(QSA);
//	grid->addLayout(ParaLay,0,0,1,2);

//	grid->setRowStretch(1,1);
	
	QPushButton* btn = new QPushButton(tr("New"));
	connect(btn,&QPushButton::clicked,this,&QParameterSet::NewParameter);
	btn->setEnabled(QCSX_Settings.GetEdit());
	grid->addWidget(btn,2,0);
	
	setLayout(grid);
}

QParameterSet::~QParameterSet()
{
}

void QParameterSet::SetModified(bool mod)
{
	emit ParameterChanged();
	ParameterSet::SetModified(mod);
}

void QParameterSet::clear()
{
	ParameterSet::clear();
	for (int i=0;i<vecQPara.size();++i)
	{
		QParameter* QPara=vecQPara.at(i);
		delete QPara;
	}
	vecQPara.clear();
}

void QParameterSet::NewParameter()
{
	QDialog* diag = new QDialog(this);
	
	QGroupBox* Group=new QGroupBox(tr("Choose Type of Parameter"));

	QVBoxLayout* lay = new QVBoxLayout();
	
	QButtonGroup* BG = new QButtonGroup();
	QRadioButton* RB = new QRadioButton("Constant");
	RB->setChecked(true);
	BG->addButton(RB,0);		
	lay->addWidget(RB);
	RB = new QRadioButton("Linear");
	BG->addButton(RB,1);		
	lay->addWidget(RB);
	
	Group->setLayout(lay);
	
	QGridLayout* Grid = new QGridLayout();
	Grid->addWidget(Group,0,0,1,2);
	
	QPushButton* PB = new QPushButton(tr("Ok"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::accept);
	Grid->addWidget(PB,1,0);
	PB = new QPushButton(tr("Cancel"));
	connect(PB,&QPushButton::clicked,diag,&QDialog::reject);
	Grid->addWidget(PB,1,1);
	
	diag->setLayout(Grid);
	diag->show();
	diag->setFixedSize(diag->size());
		
	if (diag->exec()==QDialog::Accepted) 
	{
		Parameter* newPara=nullptr;
		QParameter* QPara=nullptr;
		switch (BG->checkedId())
		{
			case 0:
				newPara = new Parameter();
				QPara = new QParameter(newPara);
				break;
			case 1:
				newPara = new LinearParameter();
				QPara = new QLinearParameter(newPara->ToLinear());
				break;				
		}
		if (newPara!=nullptr) 
		{
			if (QPara->Edit()==true) LinkParameter(newPara);
			else 
			{
				delete newPara;
				delete QPara;
			}
		}
	}
}

size_t QParameterSet::DeleteParameter(Parameter* para)
{
	if (para!=nullptr) 
	for (int i=0;i<vecQPara.size();++i)
	{
		QParameter* QPara=vecQPara.at(i);
		if (QPara->GetParameter()==para) 
		{
			delete QPara;
			vecQPara.remove(i);
		}
	}
	return ParameterSet::DeleteParameter(para);
}

void QParameterSet::AddParaWid(Parameter* newPara)
{
	if (newPara!=nullptr)
	{
		QParameter* QPara=nullptr;
		switch (newPara->GetType())
		{
			case 0:
				QPara = new QParameter(newPara);
				break;
			case 1:
				QPara = new QLinearParameter(newPara->ToLinear());
				break;				
			default:
				QPara=nullptr;
				break;
		}	
		if (QPara!=nullptr) 
		{
			ParaLay->addWidget(QPara);
			connect(QPara,&QParameter::Delete,this,&QParameterSet::DeleteParameter);	
			connect(QPara,&QParameter::ParameterChanged,this,[this](){SetModified();});	
			QPara->Update();
			vecQPara.append(QPara);
		}
	}	
}

size_t QParameterSet::LinkParameter(Parameter* newPara)
{
	AddParaWid(newPara);
	return ParameterSet::LinkParameter(newPara);
	
}
