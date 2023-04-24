import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Button, TextField, Paper,
} from '@mui/material';

import Layout from '../../components/Layout/Layout';

import * as security from '../../utils/security';
import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  root: {
    background: '#FFF5E1',
  },

  paper: {
    display: 'inlined',
    width: 540,
    marginLeft: 'auto',
    marginRight: 'auto',
    justifyContent: 'center',
    textAlign: 'center',
    paddingBottom: 50,
    paddingTop: 50,
  },

  padding: {
    height: 50,
  },

  form: {
    width: 500,
  },

  btnArea: {
    marginLeft: 40,
  },

});

export default function ChangePassword() {
  const classes = useStyles();
  const navigate = useNavigate();

  const userID = sessionStorage.getItem('userID');

  const [oldPassword, setOldPassword] = useState('');
  const [password, setPassword] = useState('');
  const [repeatPassword, setRepeatPassword] = useState('');
  const [oldPasswordError, setOldPasswordError] = useState(false);
  const [passwordError, setPasswordError] = useState(false);
  const [repeatPasswordError, setRepeatPasswordError] = useState(false);
  const [helperText, setHelperText] = useState('Change your password.');

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    }
  }, []);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setOldPasswordError(false);
    setPasswordError(false);
    setRepeatPassword(false);
    if (password !== repeatPassword) {
      setPasswordError(true);
      setRepeatPasswordError(true);
      setHelperText('Inconsistent passwords.');
    } else if (oldPassword === password) {
      setPasswordError(true);
      setRepeatPasswordError(true);
      setHelperText('New password must be different.');
    } else if (!utils.checkString(password) || !utils.checkPassword(password)) {
      setPasswordError(true);
      setRepeatPasswordError(true);
      setHelperText('Invalid password.');
    } else {
      const res = await security.changepassword(userID, oldPassword, password);
      if (res === 400) {
        setOldPasswordError(true);
        setHelperText('Wrong password. Authentication failed.');
      } else if (res === 200) {
        sessionStorage.removeItem('userID');
        navigate('/login');
      }
    }
  };

  return (
    <Layout>
      <div className={classes.root}>
        <div className={classes.padding} />
        <Paper className={classes.paper} elevation={3}>
          <Typography
            variant="h3"
            color="textPrimary"
          >
            Password
          </Typography>
          <form className={classes.form} autoComplete="off" onSubmit={handleSubmit}>
            <TextField
              sx={{
                marginTop: '20px',
                marginLeft: '20px',
              }}
              variant="outlined"
              color="secondary"
              label="Old Password"
              type="password"
              error={oldPasswordError}
              fullWidth
              inputProps={{ maxLength: 60 }}
              required
              onChange={(e) => setOldPassword(e.target.value)}
            />

            <TextField
              sx={{
                marginTop: '10px',
                marginBottom: '10px',
                marginLeft: '20px',
              }}
              variant="outlined"
              color="secondary"
              label="New Password"
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
              label="Confirm New Password"
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
                Confirm
              </Button>
            </div>
          </form>
        </Paper>
        <div className={classes.padding} />
      </div>
    </Layout>
  );
}
