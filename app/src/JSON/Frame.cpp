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

#include "JSON/Frame.h"

/**
 * Destructor function, free memory used by the @c Group objects before
 * destroying an instance of this class.
 */
JSON::Frame::~Frame()
{
  m_groups.clear();
}

/**
 * Resets the frame title and frees the memory used by the @c Group objects
 * associated to the instance of the @c Frame object.
 */
void JSON::Frame::clear()
{
  m_title = "";
  m_groups.clear();
  m_actions.clear();
}

/**
 * @brief Returns @c true if the project has a defined title and it has at least
 *        one dataset group.
 */
bool JSON::Frame::isValid() const
{
  return !title().isEmpty() && groupCount() > 0;
}

/**
 * Reads the frame information and all its asociated groups (and datatsets) from
 * the given JSON @c object.
 *
 * @return @c true on success, @c false on failure
 */
bool JSON::Frame::read(const QJsonObject &object)
{
  // Reset frame data
  clear();

  // Get title & groups array
  const auto title = object.value(QStringLiteral("title")).toString();
  const auto groups = object.value(QStringLiteral("groups")).toArray();
  const auto actions = object.value(QStringLiteral("actions")).toArray();

  // We need to have a project title and at least one group
  if (!title.isEmpty() && !groups.isEmpty())
  {
    // Update title
    m_title = title;

    // Generate groups & datasets from data frame
    for (auto i = 0; i < groups.count(); ++i)
    {
      Group group;
      if (group.read(groups.at(i).toObject()))
        m_groups.append(group);
    }

    // Generate actions from data frame
    for (auto i = 0; i < actions.count(); ++i)
    {
      Action action;
      if (action.read(actions.at(i).toObject()))
        m_actions.append(action);
    }

    // Return status
    return groupCount() > 0;
  }

  // Error
  clear();
  return false;
}

/**
 * Returns the number of groups contained in the frame.
 */
int JSON::Frame::groupCount() const
{
  return m_groups.count();
}

/**
 * Returns the title of the frame.
 */
const QString &JSON::Frame::title() const
{
  return m_title;
}

/**
 * Returns a vector of pointers to the @c Group objects associated to this
 * frame.
 */
const QVector<JSON::Group> &JSON::Frame::groups() const
{
  return m_groups;
}

/**
 * Returns a vector of pointers to the @c Action objects associated to this
 * frame.
 */
const QVector<JSON::Action> &JSON::Frame::actions() const
{
  return m_actions;
}