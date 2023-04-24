import React, { useState } from 'react';
import { makeStyles } from '@mui/styles';
import {
  Paper, Chip, Typography, Collapse, IconButton, FormControl, Select, InputLabel, Divider, Checkbox,
} from '@mui/material';

import ExpandLess from '@mui/icons-material/ExpandLess';
import ExpandMore from '@mui/icons-material/ExpandMore';

import GroupList from './GroupList';

const useStyles = makeStyles({
  groupSectionBG: {
    marginTop: 10,
    marginLeft: 20,
    minHeight: 40,
    paddingTop: 10,
    paddingBottom: 10,
  },

  padding: {
    height: 10,
  },

  TagsArea: {
    display: 'inlined',
    justifyContent: 'start',
    width: 350,
    borderRadius: 10,
    marginTop: 20,
    marginLeft: 20,
  },

  Tags: {
    display: 'flex',
    justifyContent: 'center',
  },

  selectedTags: {
    marginLeft: 10,
    marginBottom: 10,
  },

  title: {
    display: 'flex',
  },

  select1: {
    marginTop: 10,
    marginLeft: 10,
    marginBottom: 5,
  },

  select2: {
    marginTop: 10,
    marginLeft: 5,
    marginBottom: 5,
  },

});

export default function GroupBar({ groups, userInfo }) {
  const classes = useStyles();
  const allChips = ['life', 'study', 'sport', 'game', 'test'];
  const [chip, setChip] = useState(['test']);
  const [open, setOpen] = useState(false);
  const [type, setType] = useState('member');
  const [IamIn, setIamIn] = useState('N/A');
  const [order, setOrder] = useState('desc');

  const handleSelect1 = (event) => {
    setType(event.target.value);
  };

  const handleSelect2 = (event) => {
    setIamIn(event.target.value);
  };

  const handleSelect3 = (event) => {
    setOrder(event.target.value);
  };

  const handleExpand = () => {
    setOpen(!open);
  };

  const handleDelete = (chipToDelete) => () => {
    setChip((chips) => chips.filter((chip) => chip !== chipToDelete));
  };

  const handleClick = (chipToAdd) => () => {
    if (chip.indexOf(chipToAdd) < 0) {
      setChip(chip.concat(allChips.filter((chip) => chip === chipToAdd)));
    }
  };

  function sortGroup() {
    if (userInfo) {
      let temp = groups.filter((group) => chip.indexOf(group.tag) >= 0);
      if (IamIn === 'Yes') {
        temp = temp.filter((group) => userInfo.publicgroups.indexOf(group.id) >= 0);
      } else if (IamIn === 'No') {
        temp = temp.filter((group) => userInfo.publicgroups.indexOf(group.id) < 0);
      }
      if (type === 'member') {
        if (order === 'asc') {
          temp.sort((a, b) => a.members.length - b.members.length);
        } else {
          temp.sort((a, b) => b.members.length - a.members.length);
        }
      } else if (type === 'post') {
        if (order === 'asc') {
          temp.sort((a, b) => a.posts.length - b.posts.length);
        } else {
          temp.sort((a, b) => b.posts.length - a.posts.length);
        }
      }
      return temp;
    }
    const temp = groups.filter((group) => chip.indexOf(group.tag) >= 0);
    if (type === 'member') {
      if (order === 'asc') {
        temp.sort((a, b) => a.members.length - b.members.length);
      } else {
        temp.sort((a, b) => b.members.length - a.members.length);
      }
    } else if (type === 'post') {
      if (order === 'asc') {
        temp.sort((a, b) => a.posts.length - b.posts.length);
      } else {
        temp.sort((a, b) => b.posts.length - a.posts.length);
      }
    }
    return temp;
  }

  return (
    <div>
      <Paper className={classes.TagsArea}>
        <div className={classes.title}>
          <div />
          <Typography align="center" sx={{ marginLeft: '115px' }} variant="h6">
            Group Tags
          </Typography>
          <IconButton onClick={handleExpand} size="small">
            {open ? <ExpandLess /> : <ExpandMore />}
          </IconButton>
        </div>
        <Collapse in={open} timeout="auto" unmountOnExit>
          <div className={classes.Tags}>
            {allChips.map((data) => (
              <Chip
                sx={{
                  marginTop: '5px',
                  marginBottom: '5px',
                  marginLeft: '5px',
                }}
                variant="outlined"
                color="secondary"
                key={`${data}_deleted`}
                label={data}
                onClick={handleClick(data)}
              />
            ))}
          </div>
        </Collapse>
      </Paper>

      <Paper className={classes.groupSectionBG}>
        <div className={classes.selectedTags}>
          {chip.map((data) => (
            <Chip
              sx={{
                marginTop: '5px',
                marginLeft: '2px',
              }}
              variant="outlined"
              color="secondary"
              key={data}
              label={data}
              onDelete={handleDelete(data)}
            />
          ))}
        </div>
        <Divider />
        <div className={classes.title}>

          <div className={classes.select1}>
            <FormControl size="small">
              <InputLabel id="label1" color="secondary">Sorted by</InputLabel>
              <Select
                native
                labelId="label1"
                value={type}
                label="Sorted by"
                color="secondary"
                onChange={handleSelect1}
                sx={{ width: '120px' }}
              >
                <option value="member">#member</option>
                <option value="post">#post</option>
              </Select>
            </FormControl>
          </div>

          <div className={classes.select2}>
            <FormControl size="small">
              <InputLabel id="label3" color="secondary">Order</InputLabel>
              <Select
                native
                labelId="label3"
                value={order}
                label="Order"
                color="secondary"
                onChange={handleSelect3}
                sx={{ width: '100px' }}
              >
                <option value="desc">DESC</option>
                <option value="asc">ASC</option>
              </Select>
            </FormControl>
          </div>

          <div className={classes.select2}>
            <FormControl size="small">
              <InputLabel id="label2" color="secondary">I am in?</InputLabel>
              <Select
                native
                labelId="label2"
                value={IamIn}
                label="I am in?"
                color="secondary"
                onChange={handleSelect2}
                sx={{ width: '100px' }}
              >
                <option value="N/A">N/A</option>
                <option value="Yes">Yes</option>
                <option value="No">No</option>
              </Select>
            </FormControl>
          </div>
        </div>

        <div>
          <GroupList groups={sortGroup()} />
        </div>

      </Paper>

    </div>
  );
}
