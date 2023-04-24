import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Button, TextField, Paper,
} from '@mui/material';

import * as utils from '../utils/utils';
import * as fetchUser from '../utils/fetchUser';

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

export default function Register() {
  const classes = useStyles();
  const navigate = useNavigate();

  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [repeatPassword, setRepeatPassword] = useState('');
  const [usernameError, setUsernameError] = useState(false);
  const [passwordError, setPasswordError] = useState(false);
  const [repeatPasswordError, setRepeatPasswordError] = useState(false);
  const [helperText, setHelperText] = useState('Please set your username and password.');

  const handleSubmit = async (e) => {
    e.preventDefault();
    setUsernameError(false);
    setPasswordError(false);
    setRepeatPasswordError(false);
    if (!utils.checkString(username) || !utils.checkUsername(username)) {
      setUsernameError(true);
      setHelperText('Invalid username. Must be at least 3 characters and start with letter.');
    } else if (!utils.checkString(password) || !utils.checkPassword(password)) {
      setPasswordError(true);
      setRepeatPasswordError(true);
      setHelperText('Invalid password. Must be at least 8 characters and contain numbers and lowercase & uppercase letters.');
    } else if (password === repeatPassword) {
      const res = await fetchUser.register(username, password);
      if (res === 201) {
        sessionStorage.setItem('userID', username);
        navigate('/');
      } else if (res === 409) {
        setUsernameError(true);
        setHelperText('Username existed.');
      }
    } else {
      setPasswordError(true);
      setRepeatPasswordError(true);
      setHelperText('Inconsistent passwords.');
    }
  };

  return (
    <div className={classes.root}>
      <div className={classes.padding} />
      <Paper className={classes.paper} elevation={3}>
        <Link to="/">
          <img className={classes.Logo} src="Logo.svg" alt="logo" />
        </Link>
        <Typography
          variant="h3"
          color="textPrimary"
        >
          Sign Up
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
          <TextField
            sx={{
              marginBottom: '10px',
              marginLeft: '20px',
            }}
            variant="outlined"
            color="secondary"
            label="Confirm Password"
            type="password"
            error={repeatPasswordError}
            fullWidth
            inputProps={{ maxLength: 60 }}
            required
            onChange={(e) => setRepeatPassword(e.target.value)}
          />

          <Typography sx={{ marginLeft: '40px', marginBottom: '10px', color: '#9D2933' }} align="center">

            {helperText}

          </Typography>

          <div className={classes.btnArea}>
            <Button
              type="test"
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
              Sign Up
            </Button>
          </div>
        </form>

        <div className={classes.textArea}>
          <Typography
            variant="h6"
            color="textPrimary"
            align="center"
          >
            Have An Account?&ensp;
          </Typography>

          <Link to="/login">
            <Typography
              variant="h6"
              color="secondary"
              align="center"
            >
              Login
            </Typography>
          </Link>
        </div>
        <br />
      </Paper>
      <div className={classes.padding} />
    </div>
  );
}
