/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Login"

namespace Wt {
  namespace Auth {

Login::Login()
  : changed_(this), beforeChanged_(this),
    state_(LoggedOut)
{ }

void Login::login(const User& user, LoginState state)
{
  if (state == LoggedOut || !user.isValid()) {
    logout();
    return;
  } else {
    if (state != DisabledLogin && user.status() == User::Disabled)
      state = DisabledLogin;

    if (user != user_) {
      user_ = user;
	  state_ = state;
	  beforeChanged_.emit();
	  changed_.emit();
    } else if (state != state_) {
      state_ = state;
	  beforeChanged_.emit();
      changed_.emit();
    }
  }
}

void Login::logout()
{
  if (user_.isValid()) {
    user_ = User();
	state_ = LoggedOut;
	beforeChanged_.emit();
    changed_.emit();
  }
}

LoginState Login::state() const
{
  return state_;
}

bool Login::loggedIn() const
{
  return user_.isValid() && state_ != DisabledLogin;
}

const User& Login::user() const
{
  return user_;
}

  }
}
