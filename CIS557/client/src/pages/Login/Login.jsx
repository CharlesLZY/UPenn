import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Button, TextField, Paper,
} from '@mui/material';

import * as utils from '../../utils/utils';
import * as security from '../../utils/security';

const useStyles = makeStyles({
  root: {
    background: '#FFF5E1',
    minHeight: '100vh',
  },

  paper: {
    display: 'inlined',
    width: '540px',
    marginLeft: 'auto',
    marginRight: 'auto',
    justifyContent: 'center',
    textAlign: 'center',
  },

  Logo: {
    height: '200px',
  },

  form: {
    width: '500px',
  },

  btnArea: {
    marginLeft: 40,
  },

  textArea: {
    display: 'flex',
    justifyContent: 'center',
    textAlign: 'center',
    marginTop: 10,
  },

  text: {
    margin: '5px',
  },

  padding: {
    height: '50px',
  },

});

export default function Login() {
  const classes = useStyles();
  const navigate = useNavigate();

  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [usernameError, setUsernameError] = useState(false);
  const [passwordError, setPasswordError] = useState(false);
  const [helperText, setHelperText] = useState('Please enter your username and password.');

  const handleSubmit = async (e) => {
    e.preventDefault();
    setUsernameError(false);
    setPasswordError(false);
    if (!utils.checkString(username)) {
      setUsernameError(true);
      setHelperText('Invalid username or password.');
    }
    if (!password) {
      setPasswordError(true);
      setHelperText('Invalid username or password.');
    }
    if (utils.checkString(username) && utils.checkString(password)) {
      const res = await security.login(username, password);
      if (res === 200) {
        sessionStorage.setItem('userID', username);
        navigate('/');
      } else if (res === 404) {
        setUsernameError(true);
        setHelperText('Inexisted username.');
      } else if (res === 400) {
        setPasswordError(true);
        setHelperText('Wrong password.');
      } else if (res === 401) {
        setUsernameError(true);
        setPasswordError(true);
        setHelperText('Too many unsuccessful attempts. The acount is locked.');
      }
    }
  };

  return (
    <div className={classes.root}>
      <div className={classes.padding} />
      <Paper className={classes.paper} elevation={3}>

        <Link to="/">
          <img className={classes.Logo} src="Logo.svg" alt="Logo" />
        </Link>
        <Typography
          variant="h3"
          color="textPrimary"
        >
          Sign in
        </Typography>
        <form className={classes.form} autoComplete="off" onSubmit={handleSubmit}>
          <TextField
            sx={{
              marginTop: '20px',
              marginLeft: '20px',
            }}
            variant="outlined"
            color="secondary"
            label="Username"
            error={usernameError}
            fullWidth
            inputProps={{ maxLength: 60 }}
            required
            onChange={(e) => setUsername(e.target.value)}
          />

          <TextField
            sx={{
              marginTop: '10px',
              marginBottom: '10px',
              marginLeft: '20px',
            }}
            variant="outlined"
            color="secondary"
            label="Password"
            type="password"
            error={passwordError}
            fullWidth
            inputProps={{ maxLength: 60 }}
            required
            onChange={(e) => setPassword(e.target.value)}
          />

          <Typography sx={{ marginLeft: '40px', marginBottom: '10px', color: '#9D2933' }} align="center">
            {' '}
            {helperText}
            {' '}
          </Typography>

          <div className={classes.btnArea}>
            <Button
              type="submit" // must have type param!!!
              variant="outlined"
              color="secondary"
              align="center"
              sx={{
                borderRadius: '30px',
                border: '4px solid #9D2933',
                fontSize: 25,
                '&:hover': {
                  background: '#9D2933',
                  color: '#ffffff',
                  border: '4px solid #9D2933',
                },
              }}
            >
              Login
            </Button>
          </div>

        </form>

        <div className={classes.textArea}>

          <Typography
            variant="h6"
            color="textPrimary"
            align="center"
          >
            New User?&ensp;
          </Typography>

          <Link to="/register">
            <Typography
              variant="h6"
              color="secondary"
              align="center"
            >
              SIGN UP
            </Typography>
          </Link>
        </div>
        <Link to="/forget">
          <Typography
            variant="body2"
            align="center"
          >
            Forget your password?
          </Typography>
        </Link>
        <br />

      </Paper>
      <div className={classes.padding} />
    </div>

  );
}
