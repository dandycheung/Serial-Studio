/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QwtPlot>
#include <QWidget>
#include <QwtLegend>
#include <QComboBox>
#include <QVBoxLayout>
#include <QwtPlotCurve>
#include <QwtScaleEngine>

#include <UI/Dashboard.h>
#include <UI/DashboardWidget.h>

namespace Widgets
{
class MultiPlot : public QWidget
{
  Q_OBJECT

public:
  MultiPlot(const int index = -1);

private slots:
  void updateData();
  void updateRange();
  void onThemeChanged();
  void onAxisOptionsChanged();

private:
  int m_index;
  bool m_replot;
  QwtPlot m_plot;
  QwtLegend m_legend;
  QVBoxLayout m_layout;
  QVector<QwtPlotCurve *> m_curves;
  QVector<QVector<qreal>> m_yData;
};
} // namespace Widgets
