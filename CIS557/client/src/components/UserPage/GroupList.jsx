import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { FixedSizeList } from 'react-window';
import { makeStyles } from '@mui/styles';
import {
  Avatar, Paper, Typography, IconButton, InputBase, Divider,
} from '@mui/material';

import ArrowForwardIos from '@mui/icons-material/ArrowForwardIos';
import Close from '@mui/icons-material/Close';

import Icon from '../../Icon.svg';

const useStyles = makeStyles({
  searchBar: {
    height: 40,
    display: 'flex',
    justifyContent: 'space-between',
  },

  inputBar: {
    width: 250,
    marginLeft: 15,
  },

  item: {
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
  },

  searchResult: {
    '&:hover': {
      backgroundColor: '#F5AF64',
    },
    display: 'flex',
    justifyContent: 'space-between',
    height: 50,
    width: 280,
    marginLeft: 10,
  },
});

export default function GroupBar({ groups, groupType }) {
  const classes = useStyles();
  const navigate = useNavigate();

  const [input, setInput] = useState('');

  const handleReset = () => {
    document.getElementById('searchMember-inputBar').value = '';
    setInput('');
  };

  const handleClick = (group) => {
    navigate(`/${groupType}/${group}`);
  };

  function searchResult() {
    return (
      input
        ? groups.indexOf(input) >= 0
          ? (
            <Paper className={classes.searchResult}>
              <Avatar sx={{ width: '60px', marginTop: 'auto', marginBottom: 'auto' }} src={Icon} alt="Icon" />
              <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto', marginRight: '15px' }} variant="subtitle1">
                {input}
              </Typography>
              <IconButton onClick={() => handleClick(input)} sx={{ marginTop: '5px', width: 40, height: 40 }}>
                <ArrowForwardIos />
              </IconButton>
            </Paper>
          )
          : (
            <Paper className={classes.searchResult}>
              <Typography align="center" sx={{ margin: 'auto' }} variant="subtitle1">
                Not Found
              </Typography>
            </Paper>
          )
        : <div />
    );
  }

  function renderRow() {
    return (
      groups.map((group) => (
        <div key={group} className={classes.item}>
          <Avatar sx={{ width: '60px', marginTop: 'auto', marginBottom: 'auto' }} src={Icon} alt="Icon" />
          <Typography align="center" sx={{ marginTop: 'auto', marginBottom: 'auto', marginRight: '15px' }} variant="subtitle1">
            {group}
          </Typography>
          <IconButton onClick={() => handleClick(group)} sx={{ marginTop: '5px', width: 40, height: 40 }}>
            <ArrowForwardIos />
          </IconButton>
        </div>
      ))
    );
  }

  return (
    <div>
      <div className={classes.searchBar}>
        <InputBase
          className={classes.inputBar}
          placeholder="Search..."
          id="searchMember-inputBar"
          inputProps={{ maxLength: 30 }}
          onChange={(e) => setInput(e.target.value)}
        />
        <IconButton onClick={handleReset}>
          <Close />
        </IconButton>
      </div>
      {searchResult()}
      <Divider sx={{ marginTop: '10px' }} />
      <div className={classes.list}>
        <FixedSizeList
          height={250}
          width={300}
          itemSize={50}
          itemCount={1}
        >
          {renderRow}
        </FixedSizeList>
      </div>
      <Divider />
    </div>
  );
}
